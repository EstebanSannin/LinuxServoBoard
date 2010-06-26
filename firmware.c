/*
** firm.c
** 
** Made by (Esteban)
** Login   <esteban@ub32>
** 
** Started on  Thu Sep 10 17:05:51 2009 Esteban
** Last update Thu Sep 10 18:41:10 2009 Esteban
** 
** Firmware for PIC16F628 for "Robot"
**
**
**
**
*/


#define CONTROLLO_SERVO0 PORTA.F0
#define CONTROLLO_SERVO1 PORTA.F1
#define CONTROLLO_SERVO2 PORTA.F2
#define CONTROLLO_SERVO3 PORTA.F3

#define ACCENSIONE_SERVO0 PORTB.F4
#define ACCENSIONE_SERVO1 PORTB.F5
#define ACCENSIONE_SERVO2 PORTB.F6
#define ACCENSIONE_SERVO3 PORTB.F7

#define IMPULSO_MINIMO_us 500

#define SI 1
#define NO 0

#define NUMERO_SERVO 4

#define MAX_POS 255
#define MIN_POS 0

#define DIM_MASSIMA_BUFFER 6

const char stringa_Prompt[]= "\r\nSERVO> ";
const char stringa_Comando_Sconosciuto[] = "Comando sconosciuto\r\n";
const char stringa_Posizione_Cambiata[]="Servo azionato";
const char stringa_Nuova_Linea[]="\r\n";
const char stringa_Sintassi_Errata[]="Sintassi errata";
const char stringa_Stato_Cambiato[]="Stato del servo modificato";

int TEMP_REG;
int ASM_DELAY;

char car_letto;
char buffer[DIM_MASSIMA_BUFFER+1];
int cursore_buffer;
int posizioni_servo[NUMERO_SERVO];

int isInteger(char *stringa){
	int i;

	if(stringa[0]=='\0') return NO; //stringa vuota

	for(i=0;stringa[i]!='\0';i++) {
		if(stringa[i]<'0' && stringa[i]>'9') {return NO; }  }

	return SI;
}

void Usart_Write_String (const char * stringa){
	int i;

	for(i=0; stringa[i]!='\0'; i++){
		Usart_Write(stringa[i]);
	}
}

void printPrompt(){

		Usart_Write_String(stringa_Prompt);
}

void interrupt(){

	if(Usart_data_ready()){
		car_letto=Usart_Read();

		if(car_letto!=0x0D && car_letto!=0x0A && cursore_buffer<DIM_MASSIMA_BUFFER){
			Usart_Write(car_letto);
			buffer[cursore_buffer]=car_letto;
			cursore_buffer++;
		}
		else if(strlen(buffer)>0){
			buffer[cursore_buffer]='\0';
			
			cursore_buffer=0;

			if(buffer[0]=='M' && strlen(buffer)==5){
				char s_posizione[4];
				char s_numero_servo[2];
				int posizione;
				int numero_servo;

				s_numero_servo[0]=buffer[1];
				s_numero_servo[1]='\0';

				s_posizione[0]=buffer[2];
				s_posizione[1]=buffer[3];
				s_posizione[2]=buffer[4];
				s_posizione[3]='\0';

				if(isInteger(s_numero_servo) && isInteger(s_posizione)){
					posizione=atoi(s_posizione);
					numero_servo=atoi(s_numero_servo);

					if(posizione<MIN_POS) posizione=MIN_POS;
					if(posizione>MAX_POS) posizione=MAX_POS;
					if(numero_servo<0) numero_servo=0;
					if(numero_servo>3) numero_servo=3;

					posizioni_servo[numero_servo]=posizione;
				}
				else{
					Usart_Write_String(stringa_Nuova_Linea);
					Usart_Write_String(stringa_Sintassi_Errata);
				}
				Usart_Write_String(stringa_Nuova_Linea);
				Usart_Write_String(stringa_Posizione_Cambiata);
				printPrompt();
			}
			else if((buffer[0]=='O' || buffer[0]=='o') && strlen(buffer)==2){
				char s_numero_servo[2];
				int numero_servo;

				s_numero_servo[0]=buffer[1];
				s_numero_servo[1]='\0';

				if(isInteger(s_numero_servo)){
					numero_servo=atoi(s_numero_servo);

					if(buffer[0]=='O'){ //accensione servo
						switch(numero_servo){
							case 0: ACCENSIONE_SERVO0=SI;
								break;
							case 1: ACCENSIONE_SERVO1=SI;
								break;
							case 2: ACCENSIONE_SERVO2=SI;
								break;
							case 3: ACCENSIONE_SERVO3=SI;
								break;
							default: Usart_Write_String(stringa_Sintassi_Errata);
								break;
						}
					}
					else if(buffer[0]=='o'){ //spegnimento il servo
						switch(numero_servo){
							case 0: ACCENSIONE_SERVO0=NO;
								break;
							case 1: ACCENSIONE_SERVO1=NO;
								break;
							case 2: ACCENSIONE_SERVO2=NO;
								break;
							case 3: ACCENSIONE_SERVO3=NO;
								break;
							default: Usart_Write_String(stringa_Sintassi_Errata);
								break;
						}
					}
				Usart_Write_String(stringa_Nuova_Linea);
				Usart_Write_String(stringa_Stato_Cambiato);
				printPrompt();
				}
			}
			else{ //comando sconosciuto
				Usart_Write_String(stringa_Nuova_Linea);
				Usart_Write_String(stringa_Comando_Sconosciuto);
				printPrompt();
			}

		}
		else { //stringa vuota annulla
		cursore_buffer=0;
		}
	} //fine data_ready
}//fine funzione

void main(){
		int servo_corrente;
		int STEP;

		TRISA=0;
		TRISB=0;

		CMCON=0x07;

		TEMP_REG=0;
		ASM_DELAY=0;

		INTCON.GIE=1;
		PIE1.RCIE=1;
		INTCON.PEIE=1;

		Usart_init(9600);

		for(servo_corrente=0;servo_corrente<NUMERO_SERVO;servo_corrente++) {

			posizioni_servo[servo_corrente] = 128; }

		ACCENSIONE_SERVO0=SI;
		ACCENSIONE_SERVO1=SI;
		ACCENSIONE_SERVO2=SI;
		ACCENSIONE_SERVO3=SI;

		servo_corrente=0;
		cursore_buffer=0;

		while(1){
			if(servo_corrente==0){
				ASM_DELAY=posizioni_servo[0];
				CONTROLLO_SERVO0=1;
				Delay_us(IMPULSO_MINIMO_us);
				asm{
					MOVWF TEMP_REG
					INIZIO0:
						MOVLW 0X01
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						SUBWF ASM_DELAY
						BTFSS STATUS,2
						GOTO INIZIO0
					FINE0:
						MOVLW TEMP_REG
				}
				CONTROLLO_SERVO0=0;
			}
			else if (servo_corrente==1){
				ASM_DELAY=posizioni_servo[servo_corrente];
				CONTROLLO_SERVO1=1;
				Delay_us(IMPULSO_MINIMO_us);

				asm{
					MOVWF TEMP_REG
					INIZIO1:
						MOVLW 0X01
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						SUBWF ASM_DELAY
						BTFSS STATUS,2
						GOTO INIZIO1
					FINE1:
						MOVLW TEMP_REG
				}
				CONTROLLO_SERVO1=0;
			} //fine servo1
			else if (servo_corrente==2){
				ASM_DELAY=posizioni_servo[servo_corrente];
				CONTROLLO_SERVO2=1;
				Delay_us(IMPULSO_MINIMO_us);

				asm{
					MOVWF TEMP_REG
					INIZIO2:
						MOVLW 0X01
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						SUBWF ASM_DELAY
						BTFSS STATUS,2
						GOTO INIZIO2
					FINE2:
						MOVLW TEMP_REG
				}
				CONTROLLO_SERVO2=0;
			} //fine servo2
			else {
				ASM_DELAY=posizioni_servo[servo_corrente];
				CONTROLLO_SERVO3=1;
				Delay_us(IMPULSO_MINIMO_us);

				asm{
					MOVWF TEMP_REG
					INIZIO3:
						MOVLW 0X01
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						NOP
						SUBWF ASM_DELAY
						BTFSS STATUS,2
						GOTO INIZIO3
					FINE3:
						MOVLW TEMP_REG
				}
				CONTROLLO_SERVO3=0;
			} //fine servo3

		servo_corrente++;
		if(servo_corrente==NUMERO_SERVO) servo_corrente=0;
		Delay_ms(5);

	}//fine while
}//fine main
