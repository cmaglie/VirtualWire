
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

interrupt(TIMER0_A0_VECTOR) Timer_A_int(void) 
{
    vw_Int_Handler();
};

}

