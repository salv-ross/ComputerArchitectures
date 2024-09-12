/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "timer.h"
#include "../led/led.h"

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
extern unsigned char led_value;					/* defined in funct_led								*/
extern int hour, min, sec;
extern int flag_over, flag_tam, flag_sel, flag_box;
extern int happiness, satiety, x0, volume;
int second_note = 0;
extern const int freqs[8];
extern int my_freq;
int five_sec = 0;
int ticks = 0, cnt = 0;

uint16_t SinTable[45] = {     410, 467, 523, 576, 627, 673, 714, 749, 778,
															799, 813, 819, 817, 807, 789, 764, 732, 694, 
															650, 602, 550, 495, 438, 381, 324, 270, 217,
															169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
															20 , 41 , 70 , 105, 146, 193, 243, 297, 353 };

uint16_t mySinTable[45] = {   410, 467, 523, 576, 627, 673, 714, 749, 778,
															799, 813, 819, 817, 807, 789, 764, 732, 694, 
															650, 602, 550, 495, 438, 381, 324, 270, 217,
															169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
															20 , 41 , 70 , 105, 146, 193, 243, 297, 353};

void TIMER0_IRQHandler (void)
{
	/* Match register 0 interrupt service routine */
	if (LPC_TIM0->IR & 01)
	{
		if(flag_over == 0){
				if(flag_tam == 1) { flag_tam=0; }														// cambio animazione
				else if(flag_tam == 0) { flag_tam=1; }
				sec++;																											// incremento tempo
				if(sec== 60) {
					min++;
					sec = 0;
				}
				if(min==60) {
					hour++;
					min = 0;
				}	
		 five_sec++;
		 if(five_sec == 5) {																						// ogni 5 sec
					happiness--;																							// decremento happiness e satiety	
					satiety--;
					five_sec = 0;
					if(happiness == 0 || satiety == 0) {											// se uno dei due valori scende a 0, game over
						flag_over = 1;
					}
				}
		}
		LPC_TIM0->IR = 1;			/* clear interrupt flag */
	}
		/* Match register 1 interrupt service routine */
	else if(LPC_TIM0->IR & 02)
  {
		LPC_TIM0->IR =  2 ;			/* clear interrupt flag */	
	}
	/* Match register 2 interrupt service routine */
	else if(LPC_TIM0->IR & 4)
  {
		LPC_TIM0->IR =  4 ;			/* clear interrupt flag */	
	}
		/* Match register 3 interrupt service routine */
	else if(LPC_TIM0->IR & 8)
  {
		LPC_TIM0->IR =  8 ;			/* clear interrupt flag */	
	}
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
	// timer per le note, inizializzato ogni volta ad una freq diversa
			LPC_DAC->DACR = mySinTable[ticks]<<6;															// scorro la sinusoide
			ticks++;
			if(ticks == 45) {
				ticks = 0;
				cnt++;
					if(cnt == 10) {						
						cnt = 0;
						// quando finisce la prima nota, inizializza la seconda (alla freq my_freq+1)
						if(second_note == 0) { 
							second_note = 1;
							disable_timer(1);
							init_timer(1, 0, 0, 3, freqs[my_freq + 1]);
							enable_timer(1);	
						}
						// quando finisce la seconda nota, ricomincia la prima (alla freq my_freq)
						else if(second_note == 1) { 
							second_note = 0;
							disable_timer(1);
							init_timer(1, 0, 0, 3, freqs[my_freq]);
							enable_timer(1);	
						}
					}
			}
	LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
** Function name:		Timer2_IRQHandler
**
** Descriptions:		Timer/Counter 2 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER2_IRQHandler (void)
{
	// timer per l'eating 
	if(LPC_TIM2->IR & 01) {
		if(flag_box == 0 && happiness<5) { happiness++; }									// aumenta happiness o satiety in base a
		else if(flag_box == 1 && satiety<5) { satiety++; }								// quello che era stato selezionato
		x0 = 50;
		flag_sel = 0;	
	}
  LPC_TIM2->IR = 1;			/* clear interrupt flag */
  return;
}

void TIMER3_IRQHandler (void)
{
	disable_timer(1);																		// il timer 3 dovrà solo disabilitare il timer 1 e stoppare la nota riprodotta
	second_note = 0; 																		// riazzero ticks e la variabile second_note che mi servirà la prossima
	ticks = 0; 																					// volta nel timer 1 per il suono
	cnt = 0;
  LPC_TIM3->IR = 1;			/* clear interrupt flag */
  return;
}



/******************************************************************************
**                            End Of File
******************************************************************************/
