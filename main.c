#include <stdint.h>

void PLL_Init(void);
void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);
void GPIO_Init(void);
void timer_init(void);

//UART
char leUART(void);
void escreveUART(char* c);

//motor
void motorPassoInteiroHorario(uint32_t voltas);
void motorPassoInteiroAntiHorario(uint32_t voltas);
void motorMeioPassoHorario(uint32_t voltas);
void motorMeioPassoAntiHorario(uint32_t voltas);


extern int8_t aborted; 
extern int8_t running; 

	// Configuração Putty:
	// Menu Serial -> 
	// Speed(baud): 115200 
	// Data bits:8 
	// Stop bits: 1
	// Menu Session ->
	// Serial
	
int main(void){
	
	char charAux[] = "x\0";
	PLL_Init();
	SysTick_Init();
	GPIO_Init();
	timer_init();

	uint32_t i, nVoltas, sentido, velocidade, VoltasRestantes; //velocidade passo inteiro ou meio passo

	aborted = 0;
	
	//Escrevendo na tela com o UART:

	escreveUART("\n\n\n\n\n\0");
	escreveUART("MOTOR PARADO\r\n\0");
	
	//Sentido
	escreveUART("Insira o sentido de rotacao: h -> horario, a -> anti-horario\r\n\0");
	
	do{
		sentido = leUART();
	}while(!(sentido=='a'||sentido=='h'));
	
	if(sentido=='h'){
		escreveUART("\r\nSentido horario\r\n\0");
	}
	else{
		escreveUART("\r\nSentido anti-horario\r\n\0");
	}	
	
	//Número de voltas
	escreveUART("Insira o numero de voltas\r\n\0");
	
	do{
		 nVoltas = leUART();
	}while(!(nVoltas='0' && nVoltas<10));
	
	escreveUART("Num de voltas \0");
	charAux[0] = nVoltas;
	escreveUART(charAux);

	//Velocidade
	escreveUART("Insira a velocidade : m -> meio passo, p -> passo inteiro \r\n\0");
	
	do{
		velocidade = leUART();
	}while(!(velocidade=='m'||velocidade=='p'));
	
	if(velocidade=='m'){
		escreveUART("\r\nMeio passo\r\n\0");
	}
	else{
		escreveUART("\r\nPasso inteiro\r\n\0");
	}	

	escreveUART("Velocidade \0");
	charAux[0] = velocidade;
	escreveUART(charAux);
	escreveUART(" selecionada\r\n\0");
	
	//Fazendo o motor girar:
		while(1){
			
			running = 1;
				
			if(sentido=='h'){
				if(velocidade=='m'){
					motorMeioPassoHorario(nVoltas);
				}
				else{
					motorPassoInteiroHorario(nVoltas);
				}
			}
			else{
				if(velocidade=='a'){
					motorMeioPassoAntiHorario(nVoltas);
				}
				else{
					motorPassoInteiroAntiHorario(nVoltas);
				}
			}
			
			//Voltas restantes
			VoltasRestantes = nVoltas - i;
			escreveUART("\r\nVoltas restantes:\r\n\0");
			charAux[0] = VoltasRestantes;
			escreveUART(charAux);
			i--;
			
			running = 0; //?
		
		//Interrupção: para ao digitar *		
		while(leUART()!='*'){}
		
	}
					
}
