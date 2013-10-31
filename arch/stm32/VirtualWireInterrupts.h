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
    void vw_Int_Handler(); // defined below
    timer.attachCompare1Interrupt(vw_Int_Handler);

    // Refresh the timer's count, prescale, and overflow
    timer.refresh();

    // Start the timer counting
    timer.resume();
}

void vw_Int_Handler()
{
    if (vw_rx_enabled && !vw_tx_enabled)
	vw_rx_sample = digitalRead(vw_rx_pin) ^ vw_rx_inverted;
    
    // Do transmitter stuff first to reduce transmitter bit jitter due 
    // to variable receiver processing
    if (vw_tx_enabled && vw_tx_sample++ == 0)
    {
	// Send next bit
	// Symbols are sent LSB first
	// Finished sending the whole message? (after waiting one bit period 
	// since the last bit)
	if (vw_tx_index >= vw_tx_len)
	{
	    vw_tx_stop();
	    vw_tx_msg_count++;
	}
	else
	{
	    digitalWrite(vw_tx_pin, vw_tx_buf[vw_tx_index] & (1 << vw_tx_bit++));
	    if (vw_tx_bit >= 6)
	    {
		vw_tx_bit = 0;
		vw_tx_index++;
	    }
	}
    }
    if (vw_tx_sample > 7)
	vw_tx_sample = 0;
    
    if (vw_rx_enabled && !vw_tx_enabled)
	vw_pll();
}

}
