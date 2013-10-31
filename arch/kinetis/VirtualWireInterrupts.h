
// Cant really do this as a real C++ class, since we need to have 
// an ISR
extern "C"
{

#if defined(CORE_TEENSY)
  // This allows the AVR interrupt code below to be run from an
  // IntervalTimer object.  It must be above vw_setup(), so the
  // the TIMER1_COMPA_vect function name is defined.
  #ifdef SIGNAL
  #undef SIGNAL
  #endif
  #define SIGNAL(f) void f(void)
  #ifdef TIMER1_COMPA_vect
  #undef TIMER1_COMPA_vect
  #endif
  void TIMER1_COMPA_vect(void);
#endif

void vw_setup(uint16_t speed)
{
    uint16_t nticks; // number of prescaled ticks needed
    uint8_t prescaler; // Bit values for CS0[2:0]

#if defined(CORE_TEENSY)
    // on Teensy 3.0 (32 bit ARM), use an interval timer
    IntervalTimer *t = new IntervalTimer();
    t->begin(TIMER1_COMPA_vect, 125000.0 / (float)(speed));
#endif

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
#define TIMER_VECTOR TIMER1_COMPA_vect

ISR(TIMER_VECTOR)
{
    vw_Int_Handler();
}

}

