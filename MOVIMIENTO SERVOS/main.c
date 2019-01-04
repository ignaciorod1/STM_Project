/*
 Author: Tony Abboud
 
PWM_Servo Example using the Timer (TIM) peripheral
 
 Wiring Connections:
    Servo 1:
        Vcc     -> 5V
        Gnd     -> Gnd
        Signal  -> PB6 (TIM4 Ch1)
    Servo 2:
        Vcc     -> 5V
        Gnd     -> Gnd
        Signal  -> PB7 (TIM4 Ch2)

 //COMO SE PUEDE OBSERVAR UTILIZAMOS DOS CANALES DEL TIM4, DEBIDO A LA CONFIGURACIÓN DEL TIM4,
   AMBOS SERVOS TENDRÁN EL MISMO DUTY CICLE 
 
 NOTES:
    TIM4 runs on APB1 which clocks at 42MHz. If Prescaler is set to 1, then clock remains 42MHz
        Otherwise, clock is multiplied by 2 (i.e. 84MHz operation) due to PLL, See reference manual for description. This is why we prescale by 84.
 
 RESOURCES:
    - Page 214 in Ref manual (RM0090) for clock frequency description
    - Page 578 for TIM2-TIM5 information
*/
#include <stdio.h>
#include "stm32f4xx.h"

volatile uint32_t msTicks;      //counts 1ms timeTicks
void SysTick_Handler(void) {
	msTicks++;
}

//  Delays number of Systicks (happens every 1 ms)
static void Delay(__IO uint32_t dlyTicks){                                              
  uint32_t curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}

void setSysTick(void){
	// ---------- SysTick timer (1ms) -------- //
	if (SysTick_Config(SystemCoreClock / 1000)) {
		// Capture error
		while (1){};
	}
}

void config_PWM(void) {

    // Structures for configuration
    GPIO_InitTypeDef            GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;
    TIM_OCInitTypeDef           TIM_OCInitStructure;
    
    // TIM4 Clock Enable (DECLARAR EL RELOJ COMO ENABLE)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    // GPIOB Clock Enable
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    // Initalize PB6 (TIM4 Ch1) and PB7 (TIM4 Ch2)
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_100MHz;    // GPIO_High_Speed
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;         // Weak Pull-up for safety during startup
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // Assign Alternate Functions to pins
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
    
    /* Setup TIM / PWM values
     Servo Requirements:  (May be different for your servo)
        - 50Hz (== 20ms) PWM signal
        - 0.6 - 2.1 ms Duty Cycle
     
     1. Determine Required Timer_Freq.
            TIM_Period = (Timer_Freq. / PWM_Freq) - 1
     
            - We need a period of 20ms (or 20000µs) and our PWM_Freq = 50Hz (i.e. 1/20ms)
            - See NOTES, for why we use µs
            TIM_Period = 20000 - 1 = 19999  (since its 0 offset)
     
            Timer_Freq = (TIM_Period + 1) * PWM_Freq.
            Timer_Freq = (19999 + 1) * 50
            Timer_Freq = 1000000 = 1MHz
     
     2. Determine Pre-Scaler
        APB1 clock frequency:
            - SYS_CLK/4 when prescaler == 1 (i.e. 168MHz / 4 = 42MHz)
            - SYS_CLK/2 when prescaler != 1 (i.e. 168MHz / 2 = 84MHz)
     
        Prescaler = APB1_Freq / Timer_Freq
        Prescaler = 84 MHz / 1 MHz
        Prescaler = 84
     
        For our example, we can prescale the TIM clock by 84, which gives us a Timer_Freq of 1MHz
            Timer_Freq = 84 MHz / 84 = 1 MHz
        So the TIMx_CNT register will increase by 1 000 000 ticks every second. When TIMx_CNT is increased by 1 that is 1 µs. So if we want a duty cycle of 1.5ms (1500 µs) then we can set our CCRx register to 1500.
     
     NOTES:
        - TIMx_CNT Register is 16 bits, i.e. we can count from 0 to (2^16)-1 = 65535
        - If the period, TIMx_ARR, is greater than the max TIMx_CNT value (65535), then we need to choose a larger prescaler value in order to slow down the count.
        - We use the µs for a more precise adjustment of the duty cycle
     
     */
	//DECLARAMOS EL VALOR DEL PRESCALER
    uint16_t PrescalerValue = (uint16_t) 84;

    // Time Base Configuration
    TIM_TimeBaseStructure.TIM_Period        = 19999; // VALORES COMPRENDIDOS ENTRE 0-19999 (PERIODO)
    TIM_TimeBaseStructure.TIM_Prescaler     = PrescalerValue;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up; // SE PODRÍA HACER DOWN (19999-0), EN ESTE CASO IRÁ 0-19999
    
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    
    // Common TIM Settings
	// DOS MODOS:
	// PWM MODE 1: SET (ACTIVE-> HIGH)
	// PWM MODE 2: CLEAR (ACTIVE -> LOW), HABRÍA QUE CAMBIAR LA POLARIDAD A LOW, MIENTRAS NO ESTUVIESE ACTIVO ESTARÍA A HIGH.
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0;                        // Initial duty cycle
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    
	// ACTIVAR LOS DOS CANALES
    // Channel 1
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
  
    // Channel 2
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
	
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    
    // Start timer
    TIM_Cmd(TIM4, ENABLE);
}


int main(void) {
	setSysTick();
	config_PWM();
	// CCR1 (SERVO DEL CHANNEL 1) SERÁ EL SERVO DE LA BASE.
	// CCR2 (SERVO DEL CHANNEL 2) SERÁ EL SERVO DEL BRAZO.
	/* MICROSEGUNDOS-GRADOS
	             600-0º
	             750-15º
				 900-30º
				1050-45º
			    1200-60º
				1350-75º
				1500-90º
				1800-120º
				2100-150º (MAXIMO TEORICO,INTENTAREMOS LLEGAR A LOS 180º)
				2400-180º
	*/
	TIM4-> CCR2 = 1050; //CON ESTO NOS ASEGURAMOS QUE EL SERVO DEL BRAZO QUEDE A 45 GRADOS POR ENCIMA DE LA CINTA GOBERNADA POR LA FPGA.
    TIM4-> CCR1 = 600; // CON ESTO NOS ASEGURAMOS DE QUE EL BRAZO QUEDE ORIENTADO HACIA LA CINTA DE ENTREGA.
	while (1) {
		  
		 //if (BITROBOT=='1'){};
		 /*
		     do {Electroiman=='1';}
			 while ( TIM4 ->CCR1 != 2300 );
		 */
		//puesto que nuestro servo tiene unos 180 grados de giro, 
        TIM4->CCR1 = 600;       // 600 == 0.6 ms  -> 0'
        TIM4->CCR2 = 600;
        Delay(2100);
        //TIM4->CCR1 = 1500;      // 1500 == 1.5 ms -> 90'
        TIM4->CCR2 = 1200;		//BRAZO SUBE 60º
        Delay(2100);
        TIM4->CCR1 = 2400;      //BRAZO GIRA 180º
        //TIM4->CCR2 = 2100;
        Delay(2100);
		//TIM4->CCR1 = 2400;      // 2100 == 2.1 ms -> 150'
        TIM4->CCR2 = 2400;		  //BRAZO BAJA 180º HASTA LA SEGUNDA CINTA
        Delay(2100);
		//BRAZO VUELVE HACIA LA POSICION INICIAL (CINTA DE ENTREGA).
    }
	return 0;
}



