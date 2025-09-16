//Oxygen code
//#include <msp430.h>
#include "main.h"

uint16_t temp;                                          //Creates temporary holder for o2_buf[]
unsigned int ADC_Result;

void init_oxygen(){
    WDTCTL = WDTPW | WDTHOLD;                                // Stop WDT

    /* Configure GPIO
    P1DIR |= BIT0;                                           // Set P1.0/LED to output direction
    P1OUT &= ~BIT0;                                          // P1.0 LED off
    */
    
    // Configure ADC A1 pin 1.4
    P1SEL1 |= BIT4;
    P1SEL0 |= BIT4;

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure ADC
    ADCCTL0 &= ~ADCSHT_2; 
    ADCCTL0 |= ADCSHT_2 | ADCON;                             // ADCON, S&H=16 ADC clks
    //ADCCTL0 |= SREF_2;

    ADCCTL1 |= ADCSSEL_2; 
    ADCCTL1 |= ADCSHP;                                       // ADCCLK = MODOSC; sampling timer

    ADCCTL2 &= ~ADCRES;                                      // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;                                     // 12-bit conversion resolution


    ADCMCTL0 |= ADCINCH_4;                                   //Sets ADC input = A2(p1.2)  A1 ADC input select; Vref=AVCC
    

    ADCCTL0 |= ADCENC | ADCSC;  //Enable AND start conversion                          // Sampling and conversion start

    while ((ADCIFG & ADCIFG0) == 0);

        ADC_Result = ADCMEM0;       //Maximum digital value is (2^n-1)

    read_oxygen(ADC_Result);
   /* ADCCTL0 &= ~ADCENC; // Disable ADC by clearing bits
    ADCCTL0 &= ~ADCON;  // Turn off ADC by clearing bits*/

    ADCCTL0 |= ADCENC_0;                                    //Disable ADC
    /*ADCCTL0 |= ADCON_0;                                     //Turn off ADC */

    //P1SEL1 &= ~BIT4;
    //P1SEL0 &= ~BIT4;                                       //Resets Pin 1.4;
    read_oxygen(ADC_Result);
}


void read_oxygen(unsigned int data){
    int i;
    int res = 12;
    temp = data & 0xFFF;                         //Converts digital value to binary
    for(i=0; i<res; i++){
        o2_buf[0+i] = (temp>>i)&0x001;               //Shifts binary data i bits the right & saves last bit(lsb) to o2[i] in the o2 array
    }
}


// ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC_VECTOR))) ADC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            break;
        case ADCIV_ADCTOVIFG:
            break;
        case ADCIV_ADCHIIFG:
            break;
        case ADCIV_ADCLOIFG:
            break;
        case ADCIV_ADCINIFG:
            break;
        case ADCIV_ADCIFG:
            ADC_Result = ADCMEM0;
            __bic_SR_register_on_exit(LPM0_bits);            // Clear CPUOFF bit from LPM0
            break;
        default:
            break;
    }
}

