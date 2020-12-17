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
#include <time.h>
#include <stdlib.h>
/*
 * Zákaldní makra pro práci s FitKitem
 */
#define BUTTON_UP_MASK 0x4000000
#define BUTTON_LEFT_MASK 0x8000000
#define BUTTON_DOWN_MASK 0x1000
#define BUTTON_RIGHT_MASK 0x400
#define DIRECTION_RIGHT 0
#define DIRECTION_LEFT 1
#define DIRECTION_DOWN 2
#define DIRECTION_UP 3
#define CLEAR_ROW 0X0
#define SIZE_SNAKE 5
int current_size=SIZE_SNAKE;
int body_pos=0;
int cycle_number=0;
int x=2;
int y=15;
int direction=0;
struct snake{
	int x;
	int y;
};
struct snake body_arr[SIZE_SNAKE];
/*
 * Základní inicalizace hada
 */
void Init_Snake(){
	int tmp_x;
	int tmp_y;
	for(int i=0; i<SIZE_SNAKE; i++){
		if(i == 0){
			body_arr[0].x=2;
			body_arr[0].y=15;
		}else{
			if(x-i <0){
				int tmp= x-i;
				tmp_x=8-tmp;
			}else{
				tmp_x=x-i;
			}
			body_arr[i].x=tmp_x;
			body_arr[i].y=y;
		}
	}
}
#define GPIO_PIN_MASK	0x1Fu
#define GPIO_PIN(x)		(((1)<<(x & GPIO_PIN_MASK)))
/*
 * Inicializace MCU a vypnutí watchdogu
 */
void MCU_Init(void)  {
	MCG->C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
	SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}
/*
 * Inicializace všech nutných portù pro práci s FitKitem
 */
void PORTS_Init(void){
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTA_MASK;
	SIM->SCGC6   |= SIM_SCGC6_PIT_MASK;

	PORTE->PCR[10] =  ( PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01) | PORT_PCR_PE(0x01) | PORT_PCR_PS(0x01));
	PORTE->PCR[12] =  ( PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01) | PORT_PCR_PE(0x01) | PORT_PCR_PS(0x01));
	PORTE->PCR[27] =  ( PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01) | PORT_PCR_PE(0x01) | PORT_PCR_PS(0x01));
	PORTE->PCR[26] =  ( PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0x0A) | PORT_PCR_MUX(0x01) | PORT_PCR_PE(0x01) | PORT_PCR_PS(0x01));

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

	PTA->PDDR = GPIO_PDDR_PDD(0x3F000FC0);
	PTE->PDDR = GPIO_PDDR_PDD( GPIO_PIN(28) );
	NVIC_ClearPendingIRQ(PORTE_IRQn);
	NVIC_EnableIRQ(PORTE_IRQn);


}
/*
 * Funkce pro výbìr sloupcù, pøevzato z ukázky poskytnuté Ing. Václavem Šimkem a Ph.D. Vojtìchem Mrázkem
 */
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
/*
 * Výbìr jednotlivých øádkù
 */
void change_cell(int value){
	switch(value){
			case 0:
	            PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(29));
				break;
			case 1:
	            PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(27));
				break;
			case 2:
	            PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(7));
				break;
			case 3:
	            PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(28));
				break;
			case 4:
	            PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(25));
				break;
			case 5:
	            PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(9));
				break;
			case 6:
	            PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(24));
				break;
			case 7:
	            PTA->PDOR |= GPIO_PDOR_PDO( GPIO_PIN(26));
				break;
	}
}
/*
 * Funkce pro pohyb hada, jednotlivé èlánky hada se pøesouvají na svého souseda pøièemž hlava se posune daným smìrem.
 */
void move_snake(int direction){
		switch(direction){
				case DIRECTION_RIGHT:
					for(int i=SIZE_SNAKE-1; i>=0; i--){
						if(i == 0){
							body_arr[i].x+=1;
							if(body_arr[i].x == 8){
								body_arr[i].x=0;
							}
						}else{
							body_arr[i].x=body_arr[i-1].x;
							body_arr[i].y=body_arr[i-1].y;
						}
					}
					break;
				case DIRECTION_LEFT:
					for(int i=SIZE_SNAKE-1; i>=0; i--){
						if(i == 0){
							body_arr[i].x-=1;
							if(body_arr[i].x == -1){
								body_arr[i].x=7;
							}
						}else{
							body_arr[i].x=body_arr[i-1].x;
							body_arr[i].y=body_arr[i-1].y;
						}
					}
					break;
				case DIRECTION_DOWN:
					for(int i=SIZE_SNAKE-1; i>=0; i--){
						if(i == 0){
							body_arr[i].y+=1;
							if(body_arr[i].y == 16){
								body_arr[i].y=0;
							}
						}else{
							body_arr[i].x=body_arr[i-1].x;
							body_arr[i].y=body_arr[i-1].y;
						}
					}
					break;
				case DIRECTION_UP:
					for(int i=SIZE_SNAKE-1; i>=0; i--){
						if(i == 0){
							body_arr[i].y-=1;
							if(body_arr[i].y == -1){
								body_arr[i].y=15;
							}
						}else{
							body_arr[i].x=body_arr[i-1].x;
							body_arr[i].y=body_arr[i-1].y;
						}
					}
					break;
				default:
					break;
		}
}
/*
 * Pøerušení èasovaèem
 */
void PIT0_IRQHandler(void){
	PIT_TCTRL0 = 0;
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
	PTA->PDOR &= GPIO_PDOR_PDO(CLEAR_ROW);
	column_select(body_arr[body_pos].y);
	change_cell(body_arr[body_pos].x);
	if(cycle_number == 24){
		move_snake(direction);
		cycle_number=0;
		body_pos=0;
	}else{
		body_pos+=1;
	}
	if(body_pos == SIZE_SNAKE){
		body_pos=0;
		cycle_number+=1;
	}
	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK;
}
/*
 * Pøerušení pøi stisku tlaèítek
 *
 */
void PORTE_IRQHandler(void){
	 if (PORTE->ISFR & BUTTON_DOWN_MASK){
		 	 if(direction != DIRECTION_UP){
		 		 direction=DIRECTION_DOWN;
		 	 }
		 }
		 if (PORTE->ISFR & BUTTON_LEFT_MASK){
			 if(direction != DIRECTION_RIGHT){
				 direction=DIRECTION_LEFT;
			 }
		 }
		 if (PORTE->ISFR & BUTTON_UP_MASK){
			 if(direction != DIRECTION_DOWN){
				 direction=DIRECTION_UP;
			 }
		 }
		 if (PORTE->ISFR & BUTTON_RIGHT_MASK){
			 if(direction != DIRECTION_LEFT){
				 direction=DIRECTION_RIGHT;
			 }
		 }
		PORTE->ISFR = BUTTON_UP_MASK | BUTTON_LEFT_MASK | BUTTON_DOWN_MASK | BUTTON_RIGHT_MASK;
}
/*
 * Inicializace èasovaèe PIT
 */
void Init_PIT(void){
	NVIC_ClearPendingIRQ(PIT0_IRQn);
	NVIC_EnableIRQ(PIT0_IRQn);
	PIT_MCR = 0x00;
	PIT_TCTRL0 = 0x00;
	PIT_LDVAL0 = 49990;
	PIT_TCTRL0 = PIT_TCTRL_TIE_MASK;
	PIT_TFLG1 = 0x01;
	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
}
int main(void)
{

	MCU_Init();
	PORTS_Init();
	Init_Snake();
	Init_PIT();
    for (;;) {
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
