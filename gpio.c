#include <stdint.h>
#include "tm4c1294ncpdt.h"

//#define GPIO_PORTH  (0x80) //bit 8   
#define GPIO_PORTA  (0x1)  //bit 1   

void timer_init(void);
void SysTick_Wait1ms(uint32_t delay);
void motorPassoInteiroHorario(uint32_t voltas);
void motorPassoInteiroAntiHorario(uint32_t voltas);
void motorMeioPassoHorario(uint32_t voltas);
void motorMeioPassoAntiHorario(uint32_t voltas);
void Teclado_col1(void);
void Teclado_col2(void);
void Teclado_col3(void);
void Teclado_col4(void);
uint32_t Le_teclas(void);
uint32_t leTeclado(void);
void ligaLed(uint8_t leds);
void EscreveCaracter(char carac);
void EnviaComando1640(uint32_t);
uint32_t aux;
int8_t aborted, running = 0;

void timer_init(void){
	uint32_t aux;
	SYSCTL_RCGCTIMER_R = SYSCTL_RCGCTIMER_R | 0x4;
	while((SYSCTL_RCGCTIMER_R & 0x4) != 0x4 ){}
	TIMER2_CTL_R = TIMER2_CTL_R & 0xfffffffe;
	TIMER2_CFG_R = 0x00;
	aux = TIMER2_TAMR_R & 0xfffffffc;
	TIMER2_TAMR_R = aux | 0x02;
	TIMER2_TAILR_R = 16000000;
	TIMER2_TAPR_R = 0x00;
	TIMER2_ICR_R = TIMER2_ICR_R | 0x1;
	TIMER2_IMR_R = TIMER2_IMR_R | 0x1;
	NVIC_PRI5_R = 4 << 29;
	NVIC_EN0_R = 1 << 23;
	TIMER2_CTL_R = TIMER2_CTL_R | 0x1;
}


void ConfigUART(void){
	
		//Configura UART
		//Inicia clock UART0
		SYSCTL_RCGCUART_R = 0x1;
		
		while((SYSCTL_PRUART_R & 0x1) != 0x1){}
			
		UART0_CTL_R = UART0_CTL_R & 0xFFFFFFFE;
			
		UART0_IBRD_R = 520;
		UART0_FBRD_R = 0;
			
		UART0_LCRH_R = (0x3 << 5)|(0x1<<4);
		UART0_CC_R = 0x0;
			
		UART0_CTL_R = (0x1 << 9)|(0x1 << 8)|0x1;
			
			
		//Configura GPIO	
		SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R | GPIO_PORTA;
			
		while((SYSCTL_RCGCGPIO_R & (GPIO_PORTA) )!=(GPIO_PORTA)){}
			
		//GPIO UART
		GPIO_PORTA_AHB_AMSEL_R = 0x00;
			
		aux = GPIO_PORTA_AHB_PCTL_R & 0xFFFFFF00;
		GPIO_PORTA_AHB_PCTL_R = aux | 0x11;
			
		aux = GPIO_PORTA_AHB_AFSEL_R & 0xFFFFFF00;
		GPIO_PORTA_AHB_AFSEL_R = aux | 0x3;
			
		aux = GPIO_PORTA_AHB_DEN_R & 0xFFFFFF00;
		GPIO_PORTA_AHB_DEN_R = aux | 0x3;

}

void PortN_Invertepino0(void){
	if(GPIO_PORTN_DATA_R & 0x1){
		GPIO_PORTN_DATA_R = 0x0;
	}
	else if(running){
		GPIO_PORTN_DATA_R = 0x1;
	}
}


void Timer2A_Handler(void){
	TIMER2_ICR_R = 0x1;
	PortN_Invertepino0();
}

char leUART(void){
	//espera fila ter alguma coisa
	while(UART0_FR_R & (0x1 << 4)){}
		
	return((uint8_t)UART0_DR_R&0xFF);
}

void escreveUART(char* c){
	uint32_t i;
	
	for(i=0;c[i]!='\0';i++){
		while(UART0_FR_R & (0x1 << 5)){}
		UART0_DR_R = c[i];
	}
}

uint32_t leTeclado(void){
	while(1){
		Teclado_col1();
		SysTick_Wait1ms(10);
		switch(Le_teclas()){
			case 0xe:
				return(1);
			case 0xd:
				return(4);
			case 0xb:
				return(7);
			case 0x7:
				return('*');
		}
		Teclado_col2();
		SysTick_Wait1ms(10);
		switch(Le_teclas()){
			case 0xe:
				return(2);
			case 0xd:
				return(5);
			case 0xb:
				return(8);
			case 0x7:
				return(0);
		}
		Teclado_col3();
		SysTick_Wait1ms(10);
		switch(Le_teclas()){
			case 0xe:
				return(3);
			case 0xd:
				return(6);
			case 0xb:
				return(9);
			case 0x7:
				return('#');
		}
		Teclado_col4();
		SysTick_Wait1ms(10);
		switch(Le_teclas()){
			case 0xe:
				return('A');
			case 0xd:
				return('B');
			case 0xb:
				return('C');
			case 0x7:
				return('D');
		}
		SysTick_Wait1ms(100);
	}
}

void ligaLed(uint8_t leds){
	GPIO_PORTA_AHB_DATA_R = leds & 0xF0;
	GPIO_PORTQ_DATA_R = leds & 0x0F;
}

void numVoltas(int8_t voltas, int32_t i){
	int8_t j;
	int32_t resta = voltas - i;
	char text[] = " Restantes\0";
	EnviaComando1640(0xC0);
	EscreveCaracter((resta/10)+48);
	EscreveCaracter((resta%10)+48);
	
	for(j=0;text[j]!='\0';j++){
		EscreveCaracter(text[j]);
	}
}

void motorPassoInteiroHorario(uint32_t voltas){
	uint8_t leds = 0x80;
	char text[] = "Horario/inteiro\0";
	uint16_t i, j;
	for(i=0;text[i]!='\0';i++){
		EscreveCaracter(text[i]);
	}
	
	for(i=0; i<voltas; i++){
		numVoltas(voltas,i);
		for(j=0;j<512&&!aborted;j++){
			GPIO_PORTH_AHB_DATA_R = 0x08;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x04;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x02;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x01;
			SysTick_Wait1ms(5);
			if(j%64==0){
				if(leds == 0){
					leds = 0x80;
				}
				ligaLed(leds);
				leds = leds >> 1;
			}
		}
	}
	leds = 0x0;
	ligaLed(leds);
}

void motorPassoInteiroAntiHorario(uint32_t voltas){
	uint16_t leds = 0x1;
	char text[] = "Anti/inteiro\0";
	uint16_t i, j;
	for(i=0;text[i]!='\0';i++){
		EscreveCaracter(text[i]);
	}
	
	for(i=0;i<voltas;i++){
		numVoltas(voltas,i);
		for(j=0;j<512&&!aborted;j++){
			GPIO_PORTH_AHB_DATA_R = 0x01;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x02;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x04;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x08;
			SysTick_Wait1ms(5);
			if(j%64==0){
				if(leds == 0x100){
					leds = 0x1;
				}
				ligaLed((uint8_t)leds);
				leds = leds << 1;
			}
		}
	}
	leds = 0x0;
	ligaLed(leds);
}

void motorMeioPassoHorario(uint32_t voltas){
	uint8_t leds = 0x80;
	char text[] = "Horario/meio\0";
	uint16_t i, j;
	for(i=0;text[i]!='\0';i++){
		EscreveCaracter(text[i]);
	}
	
	for(i=0;i<voltas;i++){
		numVoltas(voltas,i);
		for(j=0;j<512&&!aborted;j++){
			GPIO_PORTH_AHB_DATA_R = 0x08;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x0c;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x04;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x06;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x02;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x03;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x01;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x09;
			SysTick_Wait1ms(5);
			if(j%64==0){
				if(leds == 0){
					leds = 0x80;
				}
				ligaLed(leds);
				leds = leds >> 1;
			}
		}
	}
	leds = 0x0;
	ligaLed((uint8_t)leds);
}

void motorMeioPassoAntiHorario(uint32_t voltas){
	uint16_t leds = 0x1;
	char text[] = "Anti/meio\0";
	uint16_t i, j;
	for(i=0;text[i]!='\0';i++){
		EscreveCaracter(text[i]);
	}
	
	for(i=0;i<voltas;i++){
		numVoltas(voltas,i);
		for(j=0;j<512&&!aborted;j++){
			GPIO_PORTH_AHB_DATA_R = 0x09;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x01;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x03;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x02;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x06;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x04;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x0c;
			SysTick_Wait1ms(5);
			GPIO_PORTH_AHB_DATA_R = 0x08;
			SysTick_Wait1ms(5);
			if(j%64==0){
				if(leds == 0x100){
					leds = 0x1;
				}
				ligaLed((uint8_t)leds);
				leds = leds << 1;
			}
		}
	}
	leds = 0x0;
	ligaLed((uint8_t)leds);
}

void GPIOPortJ_Handler(void){
	GPIO_PORTJ_AHB_ICR_R = 0x1;
	aborted = 1;
}
