/******************************************************************************
MSP430F2272 Project Creator 4.0

ME 461 - S. R. Platt
Fall 2010

Updated for CCSv4.2 Rick Rekoske 8/10/2011

Written by: Steve Keres
College of Engineering Control Systems Lab
University of Illinois at Urbana-Champaign
*******************************************************************************/

#include "msp430x22x2.h"
#include "UART.h"

int newprint = 0;
unsigned int timecnt = 0;

//unsigned char RXData = {0,0,0,0,0,0,0,0};
//unsigned char TXData = {0,0,0,0,0,0,0,0};

unsigned char RXsingle = 0;
unsigned char TXsingle = 0;

char rxcnt = 0;
char txcnt = 0;

int adcValue = 0;
int targetValue = 500;
int error = 0;
int error_prev = 0;
int pAdjust = 0;
int iAdjust = 0;
float pGain = 20;
float iGain = 100;
int new_output = 0;

int tune = 10;

//int cL_tune = 143;
//int cS_tune = 150;
//int d_tune = 160;
//int dS_tune = 169;
//int e_tune = 178;
//int f_tune = 188;
//int fS_tune = 200;
//int g_tune = 211;
//int gS_tune = 225;
//int a_tune = 236;
//int aS_tune = 250;
//int b_tune = 264;
//int cH_tune = 282;

int cL_tune = 558;
int cS_tune = 592;
int d_tune = 627;
int dS_tune = 664;
int e_tune = 704;
int f_tune = 746;
int fS_tune = 787;
int g_tune = 0;
int gS_tune = 0;
int a_tune = 0;
int aS_tune = 0;
int b_tune = 0;
int cH_tune = 0;

// convert note value received from orangePi to a target adc value tuned for each frequency
void setTargetValue(unsigned char noteValue) {
    if (noteValue == 0) {
        targetValue = 0;
    }
    if (noteValue == 1) {
        targetValue = cL_tune;   // cL 261.63
    }
    else if (noteValue == 2) {
        targetValue = cS_tune;   // cS 277.18
    }
    else if (noteValue == 3) {
        targetValue = d_tune;   // d 293.66
    }
    else if (noteValue == 4) {
        targetValue = dS_tune;   // dS 311.13
    }
    else if (noteValue == 5) {
        targetValue = e_tune;   // e 329.63
    }
    else if (noteValue == 6) {
        targetValue = f_tune;   // f 349.23
    }
    else if (noteValue == 7) {
        targetValue = fS_tune;   // fS 369.99
    }
    else if (noteValue == 8) {
        targetValue = g_tune;   // g 392
    }
    else if (noteValue == 9) {
        targetValue = gS_tune;   // gS 415.3
    }
    else if (noteValue == 10) {
        targetValue = a_tune;   // a 440
    }
    else if (noteValue == 11) {
        targetValue = aS_tune;   // aS 466.16
    }
    else if (noteValue == 12) {
        targetValue = b_tune;   // b 493.88
    }
    else if (noteValue == 13) {
        targetValue = cH_tune;   // cH 523.25
    }
}

void main(void) {
	
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT
	
	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);
	                                             
	DCOCTL  = CALDCO_16MHZ; // Set uC to run at approximately 16 Mhz
	BCSCTL1 = CALBC1_16MHZ; 
		
	//P1IN 		    Port 1 register used to read Port 1 pins setup as Inputs
	P1SEL &= ~0xFF; // Set all Port 1 pins to Port I/O function
	P1REN &= ~0xFF; // Disable internal resistor for all Port 1 pins
	P1DIR |= 0x1;   // Set Port 1 Pin 0 (P1.0) as an output.  Leaves Port1 pin 1 through 7 unchanged
	P1OUT &= ~0xFF; // Initially set all Port 1 pins set as Outputs to zero

    //P2IN          Port 2 register used to read Port 2 pins setup as Inputs
    P2SEL &= ~0xF0; // Set Port 2 pins 7-4 to Port I/O function
    P2REN |=  0xF0; // Enable internal resistor for Port 2 pins 7-4
    P2DIR &= ~0xF0; // Set Port 2 pins 7-4 as inputs
    P2OUT &= ~0x30; // Set port 2 pins 4,5 pull down
    P2OUT |= 0xC0;  // Set port 2 pins 6,7 pull up

    P2IE  |= 0XF0;  // enable interrupt on Port 2 pins 7-4
    P2IES &= ~0x30; // select low to high trigger on port 2 pins 4,5
    P2IES |= 0xC0;  // select high to low trigger on port 2 pins 6,7

	//P4OUT
	P4DIR |= 0x80;  // set pin 4.7 out for h-bridge direction select

    //ADC Config
    ADC10AE0 = 0x02;    // 3.1 A1
    ADC10CTL0 = SREF_0 + ADC10SHT_2 + ADC10ON + ADC10IE;
    ADC10CTL1 = ADC10SSEL_0 + SHS_0 + INCH_1 + ADC10DIV_0;

    //I2C slave
    P3SEL |= 0x06;
    UCB0CTL1 = UCSWRST + UCSSEL_2;
    UCB0CTL0 = UCMODE_3 + UCSYNC;
    UCB0I2COA = 0x25;
    UCB0CTL1 &= ~UCSWRST;
    IE2 |= UCB0RXIE;

	// Timer A Config
	TACCTL0 = CCIE;              // Enable Timer A interrupt
	TACCR0  = 16000;             // period = 1ms   
	TACTL   = TASSEL_2 + MC_1;   // source SMCLK, up mode
	
	// Timer B Config
	TBCCTL0 = 0;
	TBCCR0 = 10000;
	TBCTL = TBSSEL_2 + MC_1;

    TBCCTL1 = OUTMOD_7 + CLLD_1;
    TBCCR1 = 0;

    P4DIR |= 0x2;
    P4SEL |= 0x2;

	Init_UART(115200, 1);	// Initialize UART for 115200 baud serial communication

	_BIS_SR(GIE); 	    // Enable global interrupt

	while(1) {

		if(newmsg) {
			//my_scanf(rxbuff,&var1,&var2,&var3,&var4);
			newmsg = 0;
		}

		if (newprint==200) {
		    //UART_printf("TAR:%d\n\r", targetValue);
		    newprint = 0;
		}

		if (newprint==100)  {
			//UART_printf("R:%s\n\r", RXData);
		    UART_printf("ACT:%d - TAR:%d - ERR:%d - ADJ:%d\n\r", adcValue, targetValue, error, pAdjust+iAdjust);
		    //UART_printf("ACT:%d - P:%d - I:%d - ERR:%d\n\r", adcValue, pAdjust, iAdjust, error);
            newprint = 0;
		}

	}
}


// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
    timecnt++; // Keep track of time for main while loop.

    if ((timecnt%500) == 0) {
        //newprint = 100;  // flag main while loop that .5 seconds have gone by.
    }

    ADC10CTL0 |= ENC + ADC10SC;
}


// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    adcValue = (unsigned int)(ADC10MEM);
    error_prev = error;
    error = targetValue - adcValue;
    pAdjust = pGain * error;
    iAdjust += iGain * ((error+error_prev)/2*.001);
    if (iAdjust > 10000) {
        iAdjust = 10000;
    }
    if (iAdjust < (-1*10000)) {
        iAdjust = (-1*10000);
    }
    new_output = (int)pAdjust + (int)iAdjust;
    if (new_output < 0) {
        new_output = 0;
    }
    TBCCR1 = new_output;
}



// USCI Transmit ISR - Called when TXBUF is empty (ready to accept another character)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {

    if((IFG2&UCA0TXIFG) && (IE2&UCA0TXIE)) {        // USCI_A0 requested TX interrupt
        if(printf_flag) {
            if (currentindex == txcount) {
                senddone = 1;
                printf_flag = 0;
                IFG2 &= ~UCA0TXIFG;
            } else {
                UCA0TXBUF = printbuff[currentindex];
                currentindex++;
            }
        } else if(UART_flag)
            if(!donesending) {
                UCA0TXBUF = txbuff[txindex];
                if(txbuff[txindex] == 255) {
                    donesending = 1;
                    txindex = 0;
                } else {
                    txindex++;
                }
            }
    }

    IFG2 &= ~UCA0TXIFG;

    if((IFG2&UCB0RXIFG) && (IE2&UCB0RXIE)) {    // USCI_B0 RX interrupt occurs here for I2C
        // put your RX code here.
        //        RXData[rxcnt] = UCB0RXBUF;
        //        TXData[rxcnt] = RXData[rxcnt];
        //        rxcnt++;
        //        if (rxcnt >= 8) {
        //            P1OUT ^= 0x10;
        //            IE2 &= ~UCB0RXIE;
        //            IE2 |= UCB0TXIE;
        //            rxcnt = 0;
        //            txcnt = 0;
        //            newprint = 1;
        //        }
        RXsingle = UCB0RXBUF;
        TXsingle = RXsingle;
        P1OUT ^= 0x10;
        IE2 &= ~UCB0RXIE;
        IE2 |= UCB0TXIE;
        newprint+=5;
        //TBCCR1 = RXsingle*5*TBCCR0/100;
        setTargetValue(RXsingle);
    } else if ((IFG2&UCB0TXIFG) && (IE2&UCB0TXIE)) { // USCI_B0 TX interrupt
        // put your TX code here.
        //        UCB0TXBUF = TXData[txcnt];
        //        txcnt++;
        //        if (txcnt >= 8) {
        //            P1OUT ^= 0x20;
        //            IE2 &= ~UCB0TXIE;
        //            IE2 |= UCB0RXIE;
        //        }
        UCB0TXBUF = TXsingle;
        P1OUT ^= 0x20;
        IE2 &= ~UCB0TXIE;
        IE2 |= UCB0RXIE;
    }

}

// USCI Receive ISR - Called when shift register has been transferred to RXBUF
// Indicates completion of TX/RX operation
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
  
    if((IFG2&UCA0RXIFG) && (IE2&UCA0RXIE)) {  // USCI_A0 requested RX interrupt (UCA0RXBUF is full)

        if(!started) {  // Haven't started a message yet
            if(UCA0RXBUF == 253) {
                started = 1;
                newmsg = 0;
            }
        } else {    // In process of receiving a message
            if((UCA0RXBUF != 255) && (msgindex < (MAX_NUM_FLOATS*5))) {
                rxbuff[msgindex] = UCA0RXBUF;

                msgindex++;
            } else {    // Stop char received or too much data received
                if(UCA0RXBUF == 255) {  // Message completed
                    newmsg = 1;
                    rxbuff[msgindex] = 255; // "Null"-terminate the array
                }
                started = 0;
                msgindex = 0;
            }
        }
        IFG2 &= ~UCA0RXIFG;
    }

    if((UCB0I2CIE&UCNACKIE) && (UCB0STAT&UCNACKIFG)) { // I2C NACK interrupt

        UCB0STAT &= ~UCNACKIFG;
    }
    if((UCB0I2CIE&UCSTPIE) && (UCB0STAT&UCSTPIFG)) { // I2C Stop interrupt

        UCB0STAT &= ~UCSTPIFG;
    }
    if((UCB0I2CIE&UCSTTIE) && (UCB0STAT&UCSTTIFG)) { //  I2C Start interrupt

        UCB0STAT &= ~UCSTTIFG;
    }
    if((UCB0I2CIE&UCALIE) && (UCB0STAT&UCALIFG)) {  // I2C Arbitration Lost interrupt

        UCB0STAT &= ~UCALIFG;
    }
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    newprint=200;
    if ((P2IFG & 0x10) == 0x10) {
        targetValue+=10;
        P2IFG &= ~0x10;
    }
    if ((P2IFG & 0x20) == 0x20) {
        if (tune==10) {
            tune=1;
        }
        else if (tune==1) {
            tune=10;
        }
        P2IFG &= ~0x20;
    }
    if ((P2IFG & 0x40) == 0x40) {
        targetValue+=tune;
        P2IFG &= ~0x40;
    }
    if ((P2IFG & 0x80) == 0x80) {
        targetValue-=tune;
        P2IFG &= ~0x80;
    }
    UART_printf("ACT:%d - TAR:%d\n\r", adcValue, targetValue);
}

