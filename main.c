//*****************************************************************************
//
// MSP432 main.c - PlayWithDACs - Brian Mitchell
//
//****************************************************************************

#include <msp.h>

void SelectPortFunction(int port, int line, int sel0, int sel1) {
	if(port == 1) {
		if(P1SEL0 & BIT(line) != sel0) {
			if(P1SEL1 & BIT(line)!=sel1)
				P1SELC|=BIT(line);
			else
				P1SEL0^=BIT(line);
		} else {
			if(P1SEL1 & BIT(line)!=sel1)
				P1SEL1^=BIT(line);
		}
	} else {
		if(P2SEL0 & BIT(line)!=sel0) {
			if(P2SEL1 & BIT(line)!=sel1)
				P2SELC|=BIT(line);
			else
				P2SEL0^=BIT(line);
		} else {
			if(P2SEL1 & BIT(line)!=sel1)
				P2SEL1^=BIT(line);
		}
	}
}

void InitializePorts(void) {
	P2DIR|=(BIT6|BIT4); // set as output
	P1DIR&=~(BIT6); // set as input
	SelectPortFunction(1,6,0,0); // input
	SelectPortFunction(2,6,0,0); // clock
	SelectPortFunction(2,4,0,0); // data
	P2OUT&=~(BIT6|BIT4);
}

void SetClockFrequency(void) {
	CS->KEY=0x695A;
	CS->CTL1=0x00000233;
	CS->CLKEN=0x0000800F;
	CS->CTL0=0x00030000; //use 12 MHz clock
	CS->KEY=0xA596;
}

void ConfigureTimer(void) {
	TA0CTL=0x0200;
	TA0CCTL0=0x2000;
	TA0CCTL1=0x2010;
	TA0CCTL2=0x2010;
	TA0CCTL3=0x2010;
	TA0CCR0=0x0260;
	TA0CCR1=0x0100;
	TA0CCR2=0x0240;
	TA0CCR3=0x0248;
	TA0CTL=0x0216;
}

unsigned char activeBit = 7;
unsigned char activeByte = 0;
unsigned char data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

void setData(void) {
	if(data[activeByte] & (1 << activeBit)) {
		P2OUT&=~BIT4;
	} else {
		P2OUT|=BIT4;
	}
	if(activeBit == 0) {
		if(++activeByte == 6) {
			activeByte = 0;
		}
		activeBit = 7;
	} else {
		activeBit -= 1;
	}
}
int intCycles = 0;
void TimerA0Interrupt(void) {
	static unsigned short writeEnabled = 1;
	unsigned short intv=TA0IV;
	if(intv == 0x0E) {
		if(++intCycles == 4) {
			writeEnabled = 0;
		}
		if(intCycles == 5000) {
			intCycles = 0;
			writeEnabled = 1;
		}
	}
	if(writeEnabled) {
		if(intv == 0x02) {
			setData();
		} else if(intv == 0x04) {
			P2OUT&=~BIT6; // clock high
		} else if(intv == 0x06) {
			P2OUT|=BIT6; // clock low
		}
	}
}

void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	// Timer/init stuff
	SetClockFrequency();
	InitializePorts();
	ConfigureTimer();

	// Interrupt stuff
	NVIC_EnableIRQ(TA0_N_IRQn);

	while(1){}
}
