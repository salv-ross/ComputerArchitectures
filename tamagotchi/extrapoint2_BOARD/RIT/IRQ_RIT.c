/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../led/led.h"
#include "../GLCD/GLCD.h"
#include "../TouchPanel/TouchPanel.h"
#include "../timer/timer.h"
#include "../adc/adc.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

// variabili, alcune riazzerate nel corso del programma

// refresh per il tempo 
int refresh=0;
extern uint16_t var_color;
// vari flag per animazione tamagotchi, riquadro selezionato, se è premuto select, corri via, mangia, game over 
int flag_tam=0, flag_box = 0, flag_sel = 0, flag_over = 0, flag_eat = 0, flag_cuddle = 0;

int var_cuddle = 0;

// valori di happiness e satiety
int happiness = 5, satiety = 5;

// age tamagotchi
int hour = 0, min = 0, sec = 0;

// base del tamagotchi che verrà cambiata per spostarlo verso sx e dx
int x0 = 50;

// stringa che verrà stampata a schermo
char age_string[50];

const int freqs[8]={2120,1890,1684,1592,1417,1263,1125,1062};
// uso due note per ogni specifica
// freq[0] e freq[1] : select								durata: click (0x0007A120)
// freq[2] e freq[3] : cuddle												1s (0x017D7840)
// freq[4] e freq[5] : death												1s (0x017D7840)
// freq[6] e freq[7] : eating												1s (0x017D7840)
int my_freq = 0;

#define ORIZ 0
#define VERT 1
extern void draw_fixed(void);									// disegna schermata normale
extern void game_over(void);									// disegna schermata game over
extern void tam_running(void);								// disegna tamagotchi che corre via
void RIT_IRQHandler (void) {	
	
	disable_RIT();
	// select
	if(flag_eat == 1) {
			flag_eat = 0;
			reset_timer(1);		
			my_freq = 6;																									// timer 1: riproduce freqs[6] e freqs[7]
			init_timer(1, 0, 0, 3, freqs[my_freq]);
			enable_timer(1);																			  			
					
			reset_timer(3);																								// timer 3: durata della nota 
		  init_timer(3, 0, 0, 3, 0x017D7840);														// in questo caso dura poco perché deve simulare solo un click
		  enable_timer(3);
	}
	
	if((LPC_GPIO1->FIOPIN & (1<<25)) == 0 && flag_sel == 0 && flag_cuddle == 0){	
		  flag_sel = 1;
			x0 = 20+60*flag_box;																					// aggiorno movimento del tamagotchi, 20 se meal, 80 se snack
		  enable_timer(2);																   						// timer che dura 1s per l'eating

	  	reset_timer(1);		
			my_freq = 0;																									// timer 1: riproduce la frequenza del click
			init_timer(1, 0, 0, 3, freqs[my_freq]);
			enable_timer(1);																			  			
					
			reset_timer(3);																								// timer 3: durata della nota 
		  init_timer(3, 0, 0, 3, 0x0007A120);														// in questo caso dura poco perché deve simulare solo un click
		  enable_timer(3);
			
			flag_eat = 1;
	}
	
	// joystick a sx
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0 && flag_sel == 0 && flag_cuddle == 0){	
		flag_box = 0;	
	}
	
	// joystick a dx
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0 && flag_sel == 0 && flag_cuddle == 0){	
		flag_box = 1;
	}
	
	if(getDisplayPoint(&display, Read_Ads7846(), &matrix )){ 
	 if(display.y > 100 && display.y < 200 && display.x > 50 && display.x < 150){
			var_color = Red;
			flag_cuddle = 1;																							// faccio partire il timer che dovrà eseguire 1 nota
																																		// da qui deve iniziare a contare 2s e alla fine rimettere il colore a black
		  reset_timer(1);
		  my_freq = 2;																									// quando inizia la specifica di cuddle, verranno suonate le note 
																																		// freq[2] e freq[3]
		 	init_timer(1, 0, 0, 3, freqs[my_freq]);
			enable_timer(1);																			  			// faccio partire il timer per il cuddle (1 nota ossia freqs[1])
					
			reset_timer(3);																								// timer 3: durata suono cuddle
		  init_timer(3, 0, 0, 3, 0x017D7840);
		  enable_timer(3);
	 }
	} 
		
	if(flag_cuddle == 1) {
				var_cuddle++;
				if(var_cuddle == 40) {																			// durata di cuddle = 40 * 50ms = 2s
					var_color = Black;
					var_cuddle = 0;
					flag_cuddle = 0;
					if(happiness < 5) { happiness++; };												// incrementa happiness di 1
				}
	}
	
	ADC_start_conversion();																						// inizia conversione potenziometro

	if(flag_over==0) {																								// se non sono in game over disegna normalmente
		draw_fixed();
	}
	else if(flag_over==1) {																						// se game over la prima volta disegna il tamagotchi che corre, poi game over
		tam_running();
		flag_over = 2;
		flag_box = 2;
	}
	else if(flag_over==2) {
				game_over();																								// disegna la schermata di game over finchè non viene selezionato il RESET
				if(flag_sel == 1) {																					// è stato selezionato il RESET
					flag_over = 0;																						// devo fare il reset di tutte le variabili
					happiness = 5;
					satiety = 5;
					hour = 0;
					min = 0;
					sec = 0;
					flag_sel = 0;
					x0 = 50;
					flag_box = 0;
					flag_cuddle = 0;
					var_color = Black;
				}
	}
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
  enable_RIT();
  return;
};

/******************************************************************************
**                            End Of File
******************************************************************************/
