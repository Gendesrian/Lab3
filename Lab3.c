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
    unsigned long int temp;
    int ADC_value;
    char value[8];
    double AD_value;
    unsigned int adcBuff[16], i =0;
    unsigned int * adcPtr;

    LCDInitialize( );
    AD1PCFG &= 0xFFFE;                                                          // AN0 input pin is analog(0), rest all to digital pins(1)
    AD1CON2 = 0x003C;                                                           // sets SMPI(sample sequences per interrupt) to 1111, 16th sample/convert sequence
    AD1CON3 = 0x0D01;                                                           // set SAMC<12:8>(Auto-Sampe Time Bits(TAD)) =  13, ADCS<7:0> = 1 -> 100ns conversion time
    AD1CON1 = 0x20E4;                                                           // ADSIDL<13> = 1, SSRC<7-5> = 111(conversion trigger source select - auto convert)
    AD1CHS = 0;                                                                 // Configure input channels, connect AN0 as positive input
    AD1CSSL = 0;                                                                // No inputs are scanned
    AD1CON1bits.ADON = 1;                                                       // turn ADC on

    while(1){
        while(!IFS0bits.AD1IF);                                                 // conversion done?
        adcPtr = (unsigned int *)(&ADC1BUF0);                                   // yes
        IFS0bits.AD1IF = 0;                                                     // clear AD1IF
        temp = 0;                                                               // clear temp
        for (i=0;i<16;i++) {
            adcBuff[i] = *adcPtr++;
            temp = temp + adcBuff[i];
    }

    ADC_value = temp/16;                                                        // average the 16 ADC value
    sprintf(value, "%6d", ADC_value);                                           // copy ADC_value to value with 6 digits to value
    LCDMoveCursor(0,0);
    LCDPrintString(value);                                                      // print binary value to lcd
    AD_value = (ADC_value * 3.3)/1024;
    sprintf(value, "%6.2f", AD_value);
    LCDMoveCursor(1,0);
    LCDPrintString(value);                                                      //print digital value to lcd
    }


//    Section for PWM
//    OC1RS = ADCS;
//
//    declare some temp2
//    temp2 = ADCS;
//    temp2 = temp2 ^ 0xFFFF;
//    OC2RS = temp2;

    return 0;
}

void _ISR_ADC1Interrupt(void) {
    AD1CON1bits.ASAM = 0;
    IFS0bits.AD1IF = 0;

}