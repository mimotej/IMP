/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MK60D10.h"
#define BUTTON_UP_MASK 0x4000000
#define BUTTON_LEFT_MASK 0x8000000
#define BUTTON_DOWN_MASK 0x1000
#define BUTTON_RIGHT_MASK 0x400
#define PIEZZO_MASK 0x2000
#define LED_R_MASK 0x20
#define ROW_0 0x4000000
#define ROW_1 0x1000000
#define ROW_2 0x200
#define ROW_3 0x2000000
#define ROW_4 0x10000000
#define ROW_5 0x80
#define ROW_6 0x8000000
#define ROW_7 0x20000000
#define CLEAR_ROW 0X0
static int i = 0;
int x=2;
int y=15;
int direction_change=0;
#define GPIO_PIN_MASK	0x1Fu
#define GPIO_PIN(x)		(((1)<<(x & GPIO_PIN_MASK)))
void delay(uint64_t bound) {
	for (uint64_t i=0; i < bound; i++) { __NOP(); }
}

void MCUInit(void)  {
	MCG->C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
	SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}
void PORTSinit(void){
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTA_MASK;
	PORTB->PCR[5] =  PORT_PCR_MUX(0x01);
	PORTE->PCR[12] =   ( PORT_PCR_ISF(0x01)
			| PORT_PCR_IRQC(0x0A)
			| PORT_PCR_MUX(0x01)
			| PORT_PCR_PE(0x01)
			| PORT_PCR_PS(0x01));
	PORTE->PCR[10] =  ( PORT_PCR_ISF(0x01)| PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01)| PORT_PCR_PE(0x01)| PORT_PCR_PS(0x01));
	PORTE->PCR[27] =  ( PORT_PCR_ISF(0x01)| PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01)| PORT_PCR_PE(0x01)| PORT_PCR_PS(0x01));
	PORTE->PCR[26] =  ( PORT_PCR_ISF(0x01)| PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01)| PORT_PCR_PE(0x01)| PORT_PCR_PS(0x01));

	PORTA->PCR[8] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[10] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[6] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[11] = ( 0|PORT_PCR_MUX(0x01) );

	PORTA->PCR[26] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[24] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[9] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[25] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[28] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[7] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[27] = ( 0|PORT_PCR_MUX(0x01) );
	PORTA->PCR[29] = ( 0|PORT_PCR_MUX(0x01) );

	PORTE->PCR[28] = ( 0|PORT_PCR_MUX(0x01) );

    PTA->PDDR =  GPIO_PDDR_PDD(0x0010);
    PTB->PDDR =  GPIO_PDDR_PDD(0x3C);
	PTB->PDOR |= GPIO_PDOR_PDO(0x3C);
	PTA->PDDR = GPIO_PDDR_PDD(0x3F000FC0);
	PTE->PDDR = GPIO_PDDR_PDD( GPIO_PIN(28) );
}
void column_select(unsigned int col_num)
{
	unsigned i, result, col_sel[4];

	for (i =0; i<4; i++) {
		result = col_num / 2;
		col_sel[i] = col_num % 2;
		col_num = result;

		switch(i) {
		    case 0:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(8))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(8)));
				break;

			case 1:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(10))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(10)));
				break;


			case 2:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(6))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(6)));
				break;

			case 3:
				((col_sel[i]) == 0) ? (PTA->PDOR &= ~GPIO_PDOR_PDO( GPIO_PIN(11))) : (PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(11)));
				break;

			default:
				break;
		}
	}
}
void change_cell(int value){
	if(value > 7){
		value-=8;
	}else if(value < 0){
		value+=8;
	}
	switch(value){
			case 7:
				PTA->PDOR |= GPIO_PDOR_PDO(ROW_0);
				break;
			case 6:
				PTA->PDOR |= GPIO_PDOR_PDO(ROW_1);
				break;
			case 5:
				PTA->PDOR |= GPIO_PDOR_PDO(ROW_2);
				break;
			case 4:
				PTA->PDOR |= GPIO_PDOR_PDO(ROW_3);
				break;
			case 3:
				PTA->PDOR |= GPIO_PDOR_PDO(ROW_4);
				break;
			case 2:
				PTA->PDOR |= GPIO_PDOR_PDO(ROW_5);
				break;
			case 1:
				PTA->PDOR |= GPIO_PDOR_PDO(ROW_6);
				break;
			case 0:
				PTA->PDOR |= GPIO_PDOR_PDO(ROW_7);
				break;
	}
}
void changing_animation(int direction, int  value, int reverse){

	switch(direction){
		case 0:
			for(int a=0; a < 20; a++){
					PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);
					column_select(y+(1*value));
					change_cell(x);
					delay(5000);
					change_cell(x);
					column_select(y);
					delay(5000);
					column_select(y);
					change_cell(x+(1*reverse));
					delay(5000);
					direction_change=0;
				}
			for(int a=0; a < 20; a++){
					PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);
					column_select(y);
					change_cell(x+(2*reverse));
					delay(5000);
					change_cell(x+(1*reverse));
					column_select(y);
					delay(5000);
					change_cell(x);
					column_select(y);
					delay(5000);
					direction_change=0;
				}
			break;
		case 1:
			for(int a=0; a < 20; a++){
					PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);
					column_select(y+(1*reverse));
					change_cell(x);
					delay(5000);
					change_cell(x);
					column_select(y);
					delay(5000);
					change_cell(x+(1*value));
					column_select(y);
					delay(5000);
					direction_change=0;
				}
			for(int a=0; a < 20; a++){
					PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);
					column_select(y+(2*reverse));
					change_cell(x);
					delay(5000);
					change_cell(x);
					column_select(y+(1*reverse));
					delay(5000);
					change_cell(x);
					column_select(y);
					delay(5000);
					direction_change=0;
				}
			break;
		default:
			break;
	}
}
void move_snake(int direction){
		PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);

		switch(direction){
				case 0:
					if (direction_change==2){
							changing_animation(0, -1, 1);
						if(x== 7){
							x=1;
						}else if(x==6){
							x=0;
						}else{
							x+=2;
						}
					}else if(direction_change== 3){
							changing_animation(0, 1, 1);
							if(x== 7){
								x=1;
							}else if(x==6){
								x=0;
							}else{
								x+=2;
							}

					}
					PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);
					if(x==0){
						x+=1;
						column_select(y);
						change_cell(1);
						change_cell(0);
						change_cell(7);
						break;
					}
					if(x == 7){
						x=0;
						column_select(y);
						change_cell(0);
						change_cell(7);
						change_cell(6);
					}
					else{
						column_select(y);
						change_cell(x+1);
						change_cell(x);
						change_cell(x-1);
						x+=1;
					}
					break;
				case 1:
					if (direction_change==2){
						changing_animation(0, -1, -1);
						if(x== 0){
							x=6;
						}else if(x==1){
							x=7;
						}else{
							x-=2;
						}
					}else if(direction_change== 3){
						changing_animation(0, 1, -1);
							if(x== 0){
								x=6;
							}else if(x==0){
								x=7;
							}else{
								x-=2;
							}

					}
					PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);
					if(x==0){
						x=7;
						column_select(y);
						change_cell(1);
						change_cell(0);
						change_cell(7);
						break;
					}
					if(x == 7){
						x-=1;
						column_select(y);
						change_cell(0);
						change_cell(7);
						change_cell(6);
					}
					else{
						column_select(y);
						change_cell(x+1);
						change_cell(x);
						change_cell(x-1);
						x-=1;
					}
					break;
				case 2:
						if (direction_change==-1){
							changing_animation(1, 1, 1);
							if(y== 15){
								y=1;
							}else if(y==14){
								y=0;
							}else{
								y+=2;
							}
						}else if(direction_change== 1){
							changing_animation(1, -1, 1);
								if(y== 15){
									y=1;
								}else if(y==14){
									y=0;
								}else{
									y+=2;
								}

						}
					if(y==15){
						y=0;
						for(int a=0; a < 20; a++){
							change_cell(x);
							column_select(15);
							delay(5000);
							column_select(14);
							delay(5000);
							column_select(0);
							delay(5000);
						}
						break;
					}
					if(y == 0){
						y+=1;
						for(int a=0; a < 20; a++){
						change_cell(x);
						column_select(0);
						delay(5000);
						column_select(15);
						delay(5000);
						column_select(1);
						delay(5000);
						}
					}
					else{
						for(int a=0; a < 20; a++){
						change_cell(x);
						column_select(y+1);
						delay(5000);
						column_select(y);
						delay(5000);
						column_select(y-1);
						delay(5000);
						}
						y+=1;
					}

					break;
				case 3:
					if (direction_change==-1){
						changing_animation(1, 1, -1);
						PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);
						if(y== 1){
							y=15;
						}else if(y==0){
							y=14;
						}else{
							y-=2;
						}
					}else if(direction_change== 1){
						changing_animation(1, -1, -1);
							PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);
							if(y== 1){
								y=15;
							}else if(y==0){
								y=14;
							}else{
								y-=2;
							}

					}
					if(y==0){
						y=15;
						for(int a=0; a < 20; a++){
							change_cell(x);
							column_select(15);
							delay(5000);
							column_select(0);
							delay(5000);
							column_select(1);
							delay(5000);
						}
						break;
					}
					if(y == 15){
						y-=1;
						for(int a=0; a < 20; a++){
						change_cell(x);
						column_select(14);
						delay(5000);
						column_select(15);
						delay(5000);
						column_select(0);
						delay(5000);
						}
					}
					else{
						for(int a=0; a < 20; a++){
						change_cell(x);
						column_select(y+1);
						delay(5000);
						column_select(y);
						delay(5000);
						column_select(y-1);
						delay(5000);
						}
						y-=1;
					}
					break;
				default:
					break;
		}
}


int main(void)
{

    /* Write your code here */


	MCUInit();
	PORTSinit();
	int a=0;
	int direction=0;
	while(1){
		if(direction == 2 || direction ==3){
			move_snake(direction);
		}
		if(direction == 0 || direction == 1){
				move_snake(direction);
				delay(300000);
		}
	 if (PORTE->ISFR & BUTTON_DOWN_MASK){
		 	 if(direction == 0){
		 		 direction_change=1;
		 	 }else if(direction == 1){
		 		direction_change = -1;
		 	 }
		 	 direction=2;
		 }
		 if (PORTE->ISFR & BUTTON_LEFT_MASK){
		 	 if(direction == 2){
		 		 direction_change=2;
		 	 }else if(direction == 3){
		 		direction_change = 3;
		 	 }
		 	 direction=1;
		 }
		 if (PORTE->ISFR & BUTTON_UP_MASK){
		 	 if(direction == 0){
		 		 direction_change=1;
		 	 }else if(direction == 1){
		 		direction_change = -1;
		 	 }
		 	 direction=3;
		 }
		 if (PORTE->ISFR & BUTTON_RIGHT_MASK){
		 	 if(direction == 2){
		 		 direction_change=2;
		 	 }else if(direction == 3){
		 		direction_change = 3;
		 	 }
		 	 direction=0;
		 }
		PORTE->ISFR = BUTTON_UP_MASK | BUTTON_LEFT_MASK | BUTTON_DOWN_MASK | BUTTON_RIGHT_MASK;

	}
    /* This for loop should be replaced. By default this loop allows a single stepping. */
    for (;;) {
        i++;
    }
    /* Never leave main */
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
