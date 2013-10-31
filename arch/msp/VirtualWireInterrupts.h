
// Cant really do this as a real C++ class, since we need to have 
// an ISR
extern "C"
{

// Speed is in bits per sec RF rate
void vw_setup(uint16_t speed)
{
	// Calculate the counter overflow count based on the required bit speed
	// and CPU clock rate
	uint16_t ocr1a = (F_CPU / 8UL) / speed;
		
	// This code is for Energia/MSP430
	TA0CCR0 = ocr1a;				// Ticks for 62,5 us
	TA0CTL = TASSEL_2 + MC_1;       // SMCLK, up mode
	TA0CCTL0 |= CCIE;               // CCR0 interrupt enabled
		
	// Set up digital IO pins
	pinMode(vw_tx_pin, OUTPUT);
	pinMode(vw_rx_pin, INPUT);
	pinMode(vw_ptt_pin, OUTPUT);
	digitalWrite(vw_ptt_pin, vw_ptt_inverted);
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

interrupt(TIMER0_A0_VECTOR) Timer_A_int(void) 
{
    vw_Int_Handler();
};

}

