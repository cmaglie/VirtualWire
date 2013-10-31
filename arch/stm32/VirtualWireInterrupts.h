// Cant really do this as a real C++ class, since we need to have 
// an ISR
extern "C"
{

HardwareTimer timer(MAPLE_TIMER);

void vw_setup(uint16_t speed)
{
    // Set up digital IO pins
    pinMode(vw_tx_pin, OUTPUT);
    pinMode(vw_rx_pin, INPUT);
    pinMode(vw_ptt_pin, OUTPUT);
    digitalWrite(vw_ptt_pin, vw_ptt_inverted);

    // Pause the timer while we're configuring it
    timer.pause();
    timer.setPeriod((1000000/8)/speed);
    // Set up an interrupt on channel 1
    timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
    timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
    timer.attachCompare1Interrupt(vw_Int_Handler);

    // Refresh the timer's count, prescale, and overflow
    timer.refresh();

    // Start the timer counting
    timer.resume();
}

}
