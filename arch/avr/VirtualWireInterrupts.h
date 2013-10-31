
// Cant really do this as a real C++ class, since we need to have 
// an ISR
extern "C"
{

// Common function for setting timer ticks @ prescaler values for speed
// Returns prescaler index into {0, 1, 8, 64, 256, 1024} array
// and sets nticks to compare-match value if lower than max_ticks
// returns 0 & nticks = 0 on fault
static uint8_t _timer_calc(uint16_t speed, uint16_t max_ticks, uint16_t *nticks)
{
    // Clock divider (prescaler) values - 0/3333: error flag
    uint16_t prescalers[] = {0, 1, 8, 64, 256, 1024, 3333};
    uint8_t prescaler=0; // index into array & return bit value
    unsigned long ulticks; // calculate by ntick overflow

    // Div-by-zero protection
    if (speed == 0)
    {
        // signal fault
        *nticks = 0;
        return 0;
    }

    // test increasing prescaler (divisor), decreasing ulticks until no overflow
    for (prescaler=1; prescaler < 7; prescaler += 1)
    {
        // Amount of time per CPU clock tick (in seconds)
        float clock_time = (1.0 / (float(F_CPU) / float(prescalers[prescaler])));
        // Fraction of second needed to xmit one bit
        float bit_time = ((1.0 / float(speed)) / 8.0);
        // number of prescaled ticks needed to handle bit time @ speed
        ulticks = long(bit_time / clock_time);
        // Test if ulticks fits in nticks bitwidth (with 1-tick safety margin)
        if ((ulticks > 1) && (ulticks < max_ticks))
        {
            break; // found prescaler
        }
        // Won't fit, check with next prescaler value
    }

    // Check for error
    if ((prescaler == 6) || (ulticks < 2) || (ulticks > max_ticks))
    {
        // signal fault
        *nticks = 0;
        return 0;
    }

    *nticks = ulticks;
    return prescaler;
}

void vw_setup(uint16_t speed)
{
    uint16_t nticks; // number of prescaled ticks needed
    uint8_t prescaler; // Bit values for CS0[2:0]

#ifdef __AVR_ATtiny85__
    // figure out prescaler value and counter match value
    prescaler = _timer_calc(speed, (uint8_t)-1, &nticks);
    if (!prescaler)
    {
        return; // fault
    }

    TCCR0A = 0;
    TCCR0A = _BV(WGM01); // Turn on CTC mode / Output Compare pins disconnected

    // convert prescaler index to TCCRnB prescaler bits CS00, CS01, CS02
    TCCR0B = 0;
    TCCR0B = prescaler; // set CS00, CS01, CS02 (other bits not needed)

    // Number of ticks to count before firing interrupt
    OCR0A = uint8_t(nticks);

    // Set mask to fire interrupt when OCF0A bit is set in TIFR0
    TIMSK |= _BV(OCIE0A);

#else // ARDUINO
    // This is the path for most Arduinos
    // figure out prescaler value and counter match value
    prescaler = _timer_calc(speed, (uint16_t)-1, &nticks);    
    if (!prescaler)
    {
        return; // fault
    }

    TCCR1A = 0; // Output Compare pins disconnected
    TCCR1B = _BV(WGM12); // Turn on CTC mode

    // convert prescaler index to TCCRnB prescaler bits CS10, CS11, CS12
    TCCR1B |= prescaler;

    // Caution: special procedures for setting 16 bit regs
    // is handled by the compiler
    OCR1A = nticks;
    // Enable interrupt
#ifdef TIMSK1
    // atmega168
    TIMSK1 |= _BV(OCIE1A);
#else
    // others
    TIMSK |= _BV(OCIE1A);
#endif // TIMSK1

#endif // __AVR_ATtiny85__

    // Set up digital IO pins
    pinMode(vw_tx_pin, OUTPUT);
    pinMode(vw_rx_pin, INPUT);
    pinMode(vw_ptt_pin, OUTPUT);
    digitalWrite(vw_ptt_pin, vw_ptt_inverted);
}

// This is the interrupt service routine called when timer1 overflows
// Its job is to output the next bit from the transmitter (every 8 calls)
// and to call the PLL code if the receiver is enabled
//ISR(SIG_OUTPUT_COMPARE1A)

#undef TIMER_VECTOR
#ifdef __AVR_ATtiny85__
  #define TIMER_VECTOR TIM0_COMPA_vect
#elif defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) // Why can't Atmel make consistent?
  #define TIMER_VECTOR TIM1_COMPA_vect
#else // Assume Arduino Uno (328p or similar)
  #define TIMER_VECTOR TIMER1_COMPA_vect
#endif // __AVR_ATtiny85__

ISR(TIMER_VECTOR)
{
    vw_Int_Handler();
}

}

