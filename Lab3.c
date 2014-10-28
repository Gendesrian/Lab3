// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

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
    OC1R = 0;                                                                   // Initialize duty cycle for OC1R to 0%
    OC2R = 0;                                                                   // Initialize duty cycle for OC2R to 0%

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

    RPOR1bits.RP2R = 18;
    RPOR4bits.RP9R = 19;

    TRISAbits.TRISA3 = 0;                                                       //RB3 = pin 10
    PORTAbits.RA3 = 0;


    ///////////////////////////FIX ME/////////////////////////////////
//    ADD Change notification for button press idle, forwards, backwards

// **************************** INITIALIZE END ****************************** //



// ******************************* SAMPLING ********************************* //
    while(1){
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

//    sprintf(value, "%6.2f", AD_value);
//    LCDMoveCursor(1,0);
//    LCDPrintString(value);                                                      //print digital value to lcd
// **************************** UPDATE LCD END ****************************** //




// ******************** BUTTON PRESS STATE MACHINE ************************** //
    ///////////////////////////FIX ME/////////////////////////////////
    // modify this to use Change Notification Interrupt when written//
    //////////////////////////////////////////////////////////////////

//    switch{
//        case 0:                                                                 // DEBOUNCE/CHANGE STATE
//            if (button released){
//                if (state 1 || state 2){
//                  go to state3
//                }
//
//                if (state 3 and variable = 0){
//                  state = 1;
//                }
//
//                if (state 3 and variable = 1){
//                  state = 2;
//                }
//            break;
//
//        case 1:                                                                 // FORWARD STATE
//            if {button pressed){                                                // If button press
//              state = 0;                                                        // Go to Debounce/Change state
//            }
//            break;
//
//        case 2:                                                                 // REVERSE STATE
//            if (button pressed){                                                // If button press
//              state = 0;                                                        // Go to Debounce/Change state
//            }
//            break;
//
//        case 3:                                                                 // IDLE STATE
//            if (button pressed){                                                // If button press
//              state = 0;                                                        // Go to Debounce/Change state
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