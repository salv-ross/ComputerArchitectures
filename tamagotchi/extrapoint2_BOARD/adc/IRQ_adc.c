/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  20184-12-30
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "lpc17xx.h"
#include "../adc/adc.h"

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/

unsigned short AD_current;   
unsigned short AD_last = 0xFF;     /* Last converted value               */
extern int volume;

//const int freqs[8]={2120,1890,1684,1592,1417,1263,1125,1062};
/*
262Hz	k=2120		c4
294Hz	k=1890		
330Hz	k=1684		
349Hz	k=1592		
392Hz	k=1417		
440Hz	k=1263		
494Hz	k=1125		
523Hz	k=1062		c5

*/

extern uint16_t mySinTable[45];
extern uint16_t SinTable[45];

void ADC_IRQHandler(void) {
  	int i=0;
  AD_current = ((LPC_ADC->ADGDR>>4) & 0xFFF);/* Read Conversion Result             */
  if(AD_current != AD_last) {
		if(AD_current < 0x00F){
			volume = 0;
		}	
		else if(AD_current >= 0x00F && AD_current < 0xFFF/2) {
			volume = 1;
		}
		else if(AD_current >= 0xFFF/2 && AD_current < 0xFF0) {
			volume = 2;
		}
		else if(AD_current >= 0xFF0) {
			volume = 3;
		}
		AD_last = AD_current;
		// modifico la tabella del seno in base al volume 
		for(i=0; i<45; i++) {
				mySinTable[i] = volume * SinTable[i];
		}
	}
}
