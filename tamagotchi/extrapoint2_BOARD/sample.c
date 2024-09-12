/*----------------------------------------------------------------------------
 * Name:    sample.c
 * Purpose: to control led through EINT buttons and manage the bouncing effect
 *        	- key1 switches on the led at the left of the current led on, 
 *					- it implements a circular led effect. 	
  * Note(s): this version supports the LANDTIGER Emulator
 * Author: 	Paolo BERNARDI - PoliTO - last modified 15/12/2020
 *----------------------------------------------------------------------------
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2017 Politecnico di Torino. All rights reserved.
 *----------------------------------------------------------------------------*/
                  
#include <stdio.h>
#include "LPC17xx.H"                    /* LPC17xx definitions                */
#include "led/led.h"
#include "button_EXINT/button.h"
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "TouchPanel/TouchPanel.h"
#include "GLCD/GLCD.h"
#include "Joystick/Joystick.h"
#include "adc/adc.h"

/* Led external variables from funct_led */
#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif
/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
 
#define ORIZ 0
#define VERT 1

void square(uint16_t, uint16_t, uint16_t);
void rect(uint16_t, uint16_t, uint16_t, int, int);
void thick_line(uint16_t x0, uint16_t y0,uint16_t x1,uint16_t y1, uint16_t color, int spessore, int direzione);
void draw_box(int posizione);
void draw_batteries(void);
void draw_meal(void);
extern int x0, color = Red;
extern int hour, min, sec;
int volume = 3;
extern int happiness, satiety;
extern int flag_sel, flag_eat, flag_tam, flag_box;
extern const int freqs[8];
extern int my_freq;
uint16_t var_color = Black;
extern char age_string[50];
void game_over(void);
void tam_running(void);
void draw_beverage(void);
void red_0(int);
void red_1(int);

int main (void) {
  	
	SystemInit();  													/* System Initialization (i.e., PLL)  */
	joystick_init();
	LCD_Initialization();	
	LCD_Clear(White);												//Centro dell'LCD: 160*120
	TP_Init();
	ADC_init();
	TouchPanel_Calibrate();
	init_RIT(0x004C4B40);										/* RIT Initialization 50 msec 0x004C4B40      */

	init_timer(0, 0, 0, 3, 0x017D7840);			/* MR0 settato su 1s che alza un'interrupt */
  enable_timer(0);
	
	init_timer(2, 0, 0, 5, 0x017D7840);			/* MR2 inizializzato ad 1s */

	enable_RIT();														/* enable RIT to count 50ms				 */
	LPC_SC->PCON |= 0x1;										/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);		
	
	LPC_PINCON->PINSEL1 |= (1<<21);
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);
	
  while (1) {                           	/* Loop forever                       */	
//		__ASM("wfi");
  }

}


void square(uint16_t x, uint16_t y, uint16_t color){
		int i;
		for( i=0; i<5; i++){
			LCD_DrawLine(x,y+i,x+5,y+i, color);
		}
		return;
}

void rect(uint16_t x, uint16_t y, uint16_t color, int n, int dir){
		int i;
		if(dir==0){
			for( i=0; i<n; i++){
				square(x+i*5,y,color);
			}
		}
		else{
			for(i=0; i<n; i++){
				square(x,y+i*5,color);
			}
		}
			return;
}

void thick_line(uint16_t x0, uint16_t y0,uint16_t x1,uint16_t y1, uint16_t color, int spessore, int direzione){
	
	int i;
	for(i=0; i<spessore; i++){
			LCD_DrawLine(x0,y0, x1, y1, color);
	}
	
	if(direzione==0) {
			for(i=0; i<spessore; i++){
				LCD_DrawLine(x0,y0+i, x1, y1+i, color);
			}
		}
  else {
			for(i=0; i<spessore; i++){
				LCD_DrawLine(x0+i,y0, x1+i, y1, color);
			}
	}
	return;
}

void draw_box(int posizione){
	
			//posizione=0 riquadro a sinistra (meal)	
			//posizione=1 riquadro a destra (snack)
			thick_line(0+posizione*120,260, 120+posizione*120, 260, Red, 5,ORIZ);
			thick_line(0+posizione*120,260, 0+posizione*120, 320, Red, 5,VERT);
			thick_line(116+posizione*120,260, 116+posizione*120, 320, Red, 5,VERT);
			thick_line(0+posizione*120,316, 120+posizione*120, 316, Red, 5,ORIZ);
			return;
}

void draw_batteries(void) {
	
	thick_line(30, 45, 90, 45, Blue, 4, ORIZ);
	thick_line(30, 45, 30, 75, Blue, 4,VERT);
	thick_line(90, 45, 90, 75, Blue, 4,VERT);
	thick_line(30, 72, 90, 72, Blue, 4,ORIZ);
	thick_line(94, 55, 99, 55, Blue, 10, ORIZ);
	
	thick_line(150, 45, 210, 45, Red, 4, ORIZ);
	thick_line(150, 45, 150, 75, Red, 4,VERT);
	thick_line(210, 45, 210, 75, Red, 4,VERT);
	thick_line(150, 72, 210, 72, Red, 4,ORIZ);
	thick_line(214, 55, 219, 55, Red, 10, ORIZ);

	return;
}
void draw_levels(int x, int y, int color, int index) {
	int i=0;
	for( i=0; i<index; i++) {
	thick_line(x+i*11, y, x+i*11, y+20, color, 8, VERT);
	}
	return;
} 

void draw_volume(void) {
  int i=0;
	for(i = 0; i<10; i++) {
		thick_line(5+i, 15-i, 5+i, 15+i, Black, 1, VERT);
	}
	
	if(volume == 0) {
		thick_line(5, 20, 10, 5, Red, 5, VERT);
  }
	else {
		for(i = 0; i<volume + 1; i++) {
			thick_line(20+i*5, 10, 20+i*5, 15, Black, 1, VERT);
		}
	}
}

void draw_fixed(void) {
	
	char age_string[50];
	LCD_Clear(White);
	sprintf(age_string, "Age: %02d:%02d:%02d", hour, min, sec);
	GUI_Text(60, 6, (uint8_t *) &age_string, Black, White);
	
	draw_volume();
	// DISEGNA BATTERIE
	GUI_Text(20, 25, (uint8_t *) " HAPPINESS ", Blue, White);
	GUI_Text(150, 25, (uint8_t *) " SATIETY ", Red, White);
	
	draw_batteries();
	draw_levels(36, 50, Blue, happiness);
	draw_levels(156, 50, Red, satiety);
	
	GUI_Text(40, 280, (uint8_t *) " MEAL ", Black, White);
	GUI_Text(140, 280, (uint8_t *) " SNACK ", Black, White);
	if(flag_tam == 0) { 
			red_0(x0);
	}
	else {
			red_1(x0);
	}
	draw_box(flag_box); 
	
	if(flag_sel == 1) {			// se il select è stato premuto, in base a dov'è il riquadro devi fare cose
		if(flag_box == 0) {
		draw_meal();
		}
		if(flag_box == 1) {
		draw_beverage();
		}
	} 
}

void draw_meal(void) {
	
	thick_line(35, 234, 60, 234, Black, 5,ORIZ);
	
	thick_line(30, 230, 65, 230, Brown, 5, ORIZ);
	thick_line(30, 225, 65, 225, Brown, 5, ORIZ);
	thick_line(30, 220, 65, 220, Brown, 5, ORIZ);
	thick_line(30, 215, 65, 215, Brown, 5, ORIZ);
	thick_line(30, 210, 60, 210, Brown, 5, ORIZ);
	
	thick_line(25, 215, 25, 230, Black, 5, VERT);	
	thick_line(66, 215, 66, 230, Black, 5, VERT);
	thick_line(30, 231, 34, 231, Black, 5, ORIZ);
	thick_line(61, 231, 65, 231, Black, 5, ORIZ);
	thick_line(30, 210, 34, 210, Black, 5, ORIZ);
	thick_line(61, 210, 65, 210, Black, 5, ORIZ);
	thick_line(35, 205, 60, 205, Black, 5, ORIZ);
};

void tam_running(void) {
		int i = 0;
		reset_timer(1);
		my_freq = 4;
		init_timer(1, 0, 0, 3, freqs[my_freq]);
		enable_timer(1);																			  			// faccio partire il timer per il click (1 nota)
				
		reset_timer(3);																								// timer 3: durata della nota 
	  init_timer(3, 0, 0, 3, 0x017D7840);
	  enable_timer(3);
		for(i=0; i<5; i++) {
			LCD_Clear(White);
			red_0(x0+i*20);
		  i++;
			LCD_Clear(White);
			red_1(x0+i*20);
  	}
};

void game_over(void) {

	// schermata di reset
	LCD_Clear(White);
	sprintf(age_string, "Age: %d:%d:%d", hour, min, sec);
	GUI_Text(60, 6, (uint8_t *) &age_string, Black, White);

	GUI_Text(20, 25, (uint8_t *) " HAPPINESS ", Blue, White);
	GUI_Text(150, 25, (uint8_t *) " SATIETY ", Red, White);
	
	draw_batteries();
	draw_levels(36, 50, Blue, happiness);
	draw_levels(156, 50, Red, satiety);
	
	GUI_Text(30, 120, (uint8_t *) "Tamagotchi ran away :(", Red, White);
	GUI_Text(100, 280, (uint8_t *) " RESET ", Black, White);
	thick_line(0, 250, 240, 250, Red, 4, ORIZ);
	thick_line(0,250, 0, 320, Red, 4,VERT);
	thick_line(237 ,250, 237, 320, Red, 4,VERT);
	thick_line(0, 317, 240, 317, Red, 4,ORIZ);
	return;
};

void draw_beverage(void) {
	
	// bottom
	rect(195, 230, Black, 5,ORIZ);
	
	// water
	rect(195, 225, Blue, 5, ORIZ);
	rect(190, 220, Blue, 7, ORIZ);
	rect(190, 215, Blue, 7, ORIZ);
	rect(195, 210, Blue, 5, ORIZ);
	
	// bottle
	rect(185, 215, Black, 2, VERT);	
	rect(226, 215, Black, 2, VERT);
	square(190, 225, Black);
	square(221, 225, Black);
	square(190, 210, Black);
	square(221, 210, Black);
	rect(195, 205, Black, 5, ORIZ);
	thick_line(200, 195, 200, 205, Black, 3, VERT);
	thick_line(213, 195, 213, 205, Black, 3, VERT);
	thick_line(200, 192, 215, 192, Brown, 3, ORIZ);
	thick_line(200, 189, 215, 189, Black, 3, ORIZ); 
	thick_line(200, 186, 215, 186, Brown, 3, ORIZ);

};

void red_0(int x0) {

		rect(x0+55,110,var_color, 6, ORIZ);
		square(x0+50, 115, var_color);
		square(x0+85, 115, var_color);
	
		rect(x0+55,120,Red, 6, ORIZ);
		rect(x0+55,125,Red, 6, ORIZ);
	
		rect(x0+45,120, var_color, 5, VERT);
		rect(x0+90, 120, var_color, 5, VERT);
	
		rect(x0+40, 130, var_color, 3, ORIZ);
		rect(x0+85, 130, var_color, 3, ORIZ);
	
		rect(x0+40, 135, var_color, 2, ORIZ);
		rect(x0+90, 135, var_color, 2, ORIZ);
		rect(x0+55, 135, var_color, 6, ORIZ);

		rect(x0+35, 140, var_color, 2, VERT);
		rect(x0+100, 140, var_color, 2, VERT);
		
		rect(x0+40, 150, var_color, 2, ORIZ);
		rect(x0+90, 150, var_color, 2, ORIZ);
		
		rect(x0+40, 155, var_color, 3, ORIZ);
		rect(x0+85, 155, var_color, 3, ORIZ);
		
		// body
		rect(x0+50, 160, var_color, 8, ORIZ);
		rect(x0+50, 165, var_color, 8, ORIZ);

		// eyes
		rect(x0+60, 145, var_color, 2, VERT);
		rect(x0+75, 145, var_color, 2, VERT);
		
		rect(x0+35, 160, var_color, 3, VERT);
		rect(x0+100, 160, var_color, 2, VERT);
		rect(x0+40, 175, var_color, 3, ORIZ);
		rect(x0+85, 170, var_color, 3, ORIZ);
		square(x0+50, 170, var_color);
		rect(x0+65, 170, var_color, 2, ORIZ);
		
		// legs
		rect(x0+45, 175, var_color, 3, VERT);
		rect(x0+90, 175, var_color, 4, VERT);
		
		// shoes + pants
		rect(x0+50, 190, var_color, 3, ORIZ);
		rect(x0+75, 195, var_color, 3, ORIZ);
		rect(x0+65, 185, var_color, 2, ORIZ);
		rect(x0+60, 180, var_color, 4, ORIZ);
		square(x0+70, 190, var_color);
};

void red_1(int x0) {

		rect(x0+55,110, var_color, 6, ORIZ);
	
		square(x0+50, 115, var_color);
		square(x0+85, 115, var_color);
	
		rect(x0+55,120,Red, 6, ORIZ);
		rect(x0+55,125,Red, 6, ORIZ);
	
		rect(x0+45,120, var_color, 5, VERT);
		rect(x0+90, 120, var_color, 5, VERT);
	
		rect(x0+40, 130, var_color, 3, ORIZ);
		rect(x0+85, 130, var_color, 3, ORIZ);
	
		rect(x0+40, 135, var_color, 2, ORIZ);
		rect(x0+90, 135, var_color, 2, ORIZ);
		rect(x0+55, 135, var_color, 6, ORIZ);

		rect(x0+35, 140, var_color, 2, VERT);
		rect(x0+100, 140, var_color, 2, VERT);
		
		rect(x0+40, 150, var_color, 2, ORIZ);
		rect(x0+90, 150, var_color, 2, ORIZ);
		
		rect(x0+40, 155, var_color, 3, ORIZ);
		rect(x0+85, 155, var_color, 3, ORIZ);
		
		// body
		rect(x0+50, 160, var_color, 8, ORIZ);
		rect(x0+50, 165, var_color, 8, ORIZ);

		// eyes
		rect(x0+60, 145, var_color, 2, VERT);
		rect(x0+75, 145, var_color, 2, VERT);
		
		rect(x0+35, 160, var_color, 2, VERT);
		rect(x0+100, 160, var_color, 3, VERT);

		rect(x0+40, 170, var_color, 3, ORIZ);
		rect(x0+85, 175, var_color, 3, ORIZ);
		square(x0+85, 170, var_color);
		
		rect(x0+65, 170, var_color, 2, ORIZ);
		
		// legs
		rect(x0+45, 175, var_color, 4, VERT);
		rect(x0+90, 175, var_color, 3, VERT);
		
		// shoes + pants
		rect(x0+50, 195, var_color, 3, ORIZ);
		rect(x0+75, 190, var_color, 3, ORIZ);
		rect(x0+65, 185, var_color, 2, ORIZ);
		rect(x0+60, 180, var_color, 4, ORIZ);
		square(x0+65, 190, var_color);
};
