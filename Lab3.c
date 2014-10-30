// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

//pin10 is enable to pin1h and pin16h
//pin7 goes to pin2h
//pin6 goes to pin7h
//right wheel red to pin3h
//right wheel black to pin6h
//left wheel red to pin11h
//left wheel black to pin14h
//+5v to pin8h and pin9h
//pin21 goes to pin10h
//pin22 goes to pin15h
//ground pin4h,5h,12h,13h

#include "p24fj64ga002.h"
#include <stdio.h>
#include "lcd.h"


// ******************************************************************************************* //
// Configuration bits for CONFIG1 settings.
//
// Make sure "Configuration Bits set in code." option is checked in MPLAB.
//
// These settings are appropriate for debugging the PIC microcontroller. If you need to
// program the PIC for standalone operation, change the COE_ON option to COE_OFF.

_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF &
		 BKBUG_ON & COE_ON & ICS_PGx1 &
		 FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )

// ******************************************************************************************* //
// Configuration bits for CONFIG2 settings.
// Make sure "Configuration Bits set in code." option is checked in MPLAB.

_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
		 IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )

                                                                  // forward 1 backwards 0
    volatile int state = 0;
    volatile int lastState = 1;
    volatile int flag = 1;

int main(void){

// ****************************** INITIALIZE ******************************** //

    unsigned long int temp;                                                     // Initialize temp variable for calc ADC_value
    int ADC_value;                                                              // Initialize integer to hold average of adcBuff sampling
    char value1[8];                                                             // Initialize string for ADC_value storage
    char value2[8];                                                             // Initialize string for oc1Temp storage
    char value3[8];                                                             // Initialize string for oc2Temp storage
    double AD_value;
    unsigned int adcBuff[16], i =0;                                             // Initalize buffer for sampling
    unsigned int * adcPtr;
    unsigned int oc1Temp;                                                       // Initialize variable for duty cycle LCD dispay
    unsigned int oc2Temp;                                                       // Initialize variable for duty cycle LCD display
    
    OC1CON = 0x000E;                                                            // Set OC1CON to PWM w/o protection
    OC2CON = 0x000E;                                                            // Set OC2CON to PWM w/o protection
    OC3CON = 0x000E;
    OC5CON = 0x000E;
    OC1R = 0;                                                                   // Initialize duty cycle for OC1R to 0%
    OC2R = 0;                                                                   // Initialize duty cycle for OC2R to 0%
    OC3R = 0;
    OC5R = 0;

    T3CON = 0x8020;                                                             // Turn on and set prescaler to 64
    TMR3 = 0;
    PR3 = 512;                                                                  // 2.2 ms timer

    IFS0bits.T3IF = 0;                                                          // Put down interupt flag
    IEC0bits.T3IE = 0;                                                          // Do not enable ISR

    LCDInitialize( );
    AD1PCFG &= 0xFFFE;                                                          // AN0 input pin is analog(0), rest all to digital pins(1)
    AD1CON2 = 0x003C;                                                           // Sets SMPI(sample sequences per interrupt) to 1111, 16th sample/convert sequence
    AD1CON3 = 0x0D01;                                                           // Set SAMC<12:8>(Auto-Sampe Time Bits(TAD)) =  13, ADCS<7:0> = 1 -> 100ns conversion time
    AD1CON1 = 0x20E4;                                                           // ADSIDL<13> = 1, SSRC<7-5> = 111(conversion trigger source select - auto convert)
    AD1CHS = 0;                                                                 // Configure input channels, connect AN0 as positive input
    AD1CSSL = 0;                                                                // No inputs are scanned
    AD1CON1bits.ADON = 1;                                                       // Turn ADC on

//    RPOR1bits.RP2R = 18;                                                        //pin6 used for OC1
//    RPOR1bits.RP3R = 20;                                                        //pin7 used for OC1 ground
//
//    RPOR5bits.RP10R = 19;                                                        //pin21 used for OC2
//    RPOR5bits.RP11R = 20;                                                        //pin22 used for OC2 ground

    TRISAbits.TRISA3 = 0;                                                       //RB3 = pin 10 as output
    TRISBbits.TRISB5 = 1;                                                       //RB5 is input, SW5 on PIC
    //PORTBbits.RB5 = 1;

    CNEN2bits.CN27IE = 1;                                                        //enable change notification for RB5


    IFS1bits.CNIF = 0;                                                          //drop the CN flag
    IEC1bits.CNIE = 1;                                                          //allow change notifications

    ///////////////////////////FIX ME/////////////////////////////////
//    ADD Change notification for button press idle, forwards, backwards

// **************************** INITIALIZE END ****************************** //




    while(1){

        switch (state){


                case 0:                                                         // Pulse State


// ******************************* SAMPLING ********************************* //
        while(!IFS0bits.AD1IF);                                                 // While conversion not done
        adcPtr = (unsigned int *)(&ADC1BUF0);                                   // yes
        IFS0bits.AD1IF = 0;                                                     // Clear AD1IF
        temp = 0;                                                               // Clear temp
        for (i=0;i<16;i++) {                                                    // Iterrate to sum up adcBuff
            adcBuff[i] = *adcPtr++;
            temp = temp + adcBuff[i];   
        }
                     
    ADC_value = temp/16;                                                        // Average the 16 ADC value = binary->decimal conversion

// ***************************** SAMPLING END ******************************* //



// ***************************** CALCULATIONS ******************************* //
    OC1RS =  1023 - ADC_value;                                                  // Load OC1RS buffer with the opposite of OC2RS
    OC2RS = ADC_value;                                                          // Load OC2RS buffer with the value of ADC_value

    AD_value = (ADC_value * 3.3)/1024;

    if (ADC_value >= 512 ){                                                     // If pot more than 1/2
    oc1Temp = (OC1RS * 100) / 512;                                              // Calculate duty cycle of OC1RS and store to oc1Temp
    oc2Temp = 100;                                                              // Store duty cycle of 100% to oc2Temp
    }

    if (ADC_value <= 511){                                                      // If pot lesss than 1/2
        oc1Temp = 100;                                                          // Store dutycycle of 100% to oc1Temp
        oc2Temp = (OC2RS * 100) / 512;                                          // Calculate duty cycle of OC2RS and store to oc2Temp
    }
// *************************** CALCULATIONS END ***************************** //

 



// ****************************** UPDATE LCD ******************************** //
    sprintf(value1, "%6d", ADC_value);                                          // Copy ADC_value to value1 with 6 digits to value1
    LCDMoveCursor(0,0);                                                         // Move cursor to start of line 1
    LCDPrintString(value1);                                                     // Print value2 to the lcd

    sprintf(value2, "%3d", oc1Temp);                                            // Copy oc1Temp to value2 with 3 digits to value2
    LCDMoveCursor(1,0);                                                         // Move cursor to start of line 2
    LCDPrintString(value2);                                                     // Print value2 to the lcd

    sprintf(value3, "%3d", oc2Temp);                                            // Copy oc2Temp to value3 with 3 digits to value3
    LCDMoveCursor(1,5);                                                         // Move cursor to middle of line 2
    LCDPrintString(value3);                                                     // Print value3 to the lcd

                                                 
// **************************** UPDATE LCD END ****************************** //

             break;



            case 1:                                                             // Forward State

            PORTAbits.RA3 = 0;                                                  // Turn H-Bridge on
            RPOR1bits.RP2R = 18;                                                //pin6 used for OC1
            RPOR1bits.RP3R = 20;                                                //pin7 used for OC1 ground

            RPOR5bits.RP10R = 19;                                               //pin21 used for OC2
            RPOR5bits.RP11R = 20;                                               //pin22 used for OC2 ground
            lastState = 3;                                                      // last state was forward
            flag = 0;
            state = 0;
                break;



            case 2:                                                             // Reverse State

            PORTAbits.RA3 = 0;                                                  // Turn H-Bridge on
            RPOR1bits.RP2R = 20;                                              //pin6 used for OC1
            RPOR1bits.RP3R = 18;                                              //pin7 used for OC1 ground

            RPOR5bits.RP10R = 20;                                             //pin21 used for OC2
            RPOR5bits.RP11R = 19;                                             //pin22 used for OC2 ground
            lastState = 3;
            flag = 1;
            state = 0;

            break;


            case 3:                                                             // Idle State
                OC1RS = 0;
                OC2RS = 0;
//            PORTAbits.RA3 = 1;                                                  // Turn H-Bridge off
           //state = 0;
            if (flag == 0){
                lastState = 2;
            }
            if (flag == 1){
                lastState =1;
            }

            break;

            case 4:                                                             // Debounce
                if(PORTBbits.RB5 == 1){
                    state = lastState;
                }
                break;






        }



// ******************** BUTTON PRESS STATE MACHINE ************************** //
    ///////////////////////////FIX ME/////////////////////////////////
    // modify this to use Change Notification Interrupt when written//
    //////////////////////////////////////////////////////////////////

//    switch(state)
//        case 0:                                                                 // DEBOUNCE1
//            state = 1;
//            break;
//
//        case 2:                                                                 // FORWARD STATE
//            state = 0;
//            RPOR1bits.RP2R = 18;                                           //pin6 used for OC1
//            RPOR1bits.RP3R = 20;                                           //pin7 used for OC1 ground
//
//            RPOR5bits.RP10R = 19;                                            //pin21 used for OC2
//            RPOR5bits.RP11R = 20;                                           //pin22 used for OC2 ground
//            lastState = 1;                                                    // last state was forward
//            break;
//
//            case 3:
//
//        case 4:                                                                 // REVERSE STATE
//
//              state = 0;                                                        // Go to Debounce/Change state
//                RPOR1bits.RP2R = 20;                                              //pin6 used for OC1
//                RPOR1bits.RP3R = 18;                                              //pin7 used for OC1 ground
//
//                RPOR5bits.RP10R = 20;                                             //pin21 used for OC2
//                RPOR5bits.RP11R = 19;                                             //pin22 used for OC2 ground
//               lastState = 0;
//            }
//            break;
//
//        case 5:                                                                 // IDLE STATE
//                OC1R = 0;                                                                   // Initialize duty cycle for OC1R to 0%
//                OC2R = 0;
//            break;
//
//    }


    }
    



 

    return 0;
}

void _ISR_ADC1Interrupt(void) {
    AD1CON1bits.ASAM = 0;
    IFS0bits.AD1IF = 0;

}

void __attribute__((interrupt)) _T3Interrupt(void){
    IFS0bits.T3IF = 0;

}

void __attribute__((interrupt)) _CNInterrupt(void){
    IFS1bits.CNIF = 0;
    state = 4;

//    if(state == 3 && PORTBbits.RB5 == 1 && lastState == 1)
//    {
//        state = 2;
//    }
//
//     if(state == 3 && PORTBbits.RB5 == 1 && lastState == 0)
//    {
//        state = 0;
//
//    }
//
//    if(state == 1 && PORTBbits.RB5 == 1)
//    {
//        state = 0;
//    }

}