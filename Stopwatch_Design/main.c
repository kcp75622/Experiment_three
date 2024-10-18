/*
 * @file main.c
 *
 * @brief Main source code for the Stopwatch_Design program.
 *
 * This file contains the main entry point and function definitions for the Stopwatch_Design program.
 * This lab involves designing a basic stopwatch. It demonstrates edge-triggered interrupts, 
 * and it interfaces with the following:
 *  - User LED (RGB) Tiva C Series TM4C123G LaunchPad
 *	- EduBase Board LEDs (LED0 - LED3)
 *	- EduBase Board Push Buttons (SW2 - SW3)
 *	- EduBase Board Seven-Segment Display
 *	- PMOD BTN module
 *
 * It configures the pins used by the EduBase Board push buttons (SW2 and SW3) and the PMOD
 * BTN module to generate interrupts on rising edges.
 * 
 * Timer 0A is used to generate periodic interrupts every 1 millisecond. The values of the 
 * stopwatch (milliseconds, seconds, and minutes) will increment in the Timer 0A
 * periodic task. The PMOD BTN module will be used to control the stopwatch.
 *
 * @Katherine Poz
 */
#include "TM4C123GH6PM.h"
#include "GPIO.h"
#include "PMOD_BTN_Interrupt.h"
#include "EduBase_Button_Interrupt.h"
#include "Seven_Segment_Display.h"
#include "Timer_0A_Interrupt.h"

//Declare the user-defined function prototype for PMOD_BTN_Interrupt
void PMOD_BTN_Handler(uint8_t pmod_btn_status);

//Declare the user-defined function prototype for EduBase_Button_Interrupt
void EduBase_Button_Handler(uint8_t edubase_button_status);

//Initialize a global variable for an 8-bit counter
static uint8_t counter = 0; 

// * Declare the function prototype for the function that calculates
//stopwatch value and stores it in an array
void Calculate_Stopwatch_Value(uint8_t stopwatch_value[]);

// Declare the function prototype for the user-defined function for Timer 0A
void Timer_0A_Periodic_Task(void);

// Initialize a global variable for Timer 0A to keep track of elapsed time in milliseconds
static uint8_t ms_elapsed = 0;

// Initialize global variable to keep track of the stopwatch value (i.e. milliseconds, seconds, and minutes)
// "milliseconds" is updated every 100 ms (Range 0 to 999 ms)
// "seconds" is updated every 1000 ms (Range 0 to 59 sec)
// "minutes" is updated every 60 seconds (Range 0 to 9 minutes)
static uint8_t milliseconds = 0;
static uint8_t seconds = 0;
static uint8_t minutes = 0;

// Initialize global flags for starting and resetting the stopwatch
static uint8_t start_stopwatch = 0;
static uint8_t reset_stopwatch = 0;

int main(void)
{
	// Initialize the push buttons on the PMOD BTN module (Port A)
	PMOD_BTN_Interrupt_Init(&PMOD_BTN_Handler);
	
	// Initialize the LEDs on the EduBase board (Port B)
	EduBase_LEDs_Init();
	
	// Initialize the SysTick timer used to provide blocking delay functions
	SysTick_Delay_Init();
	
	// Initialize the Seven Segment Display (Port B and C)
	Seven_Segment_Display_Init();
	
	// Initialize the SW2 and SW3 on the EduBase board with interrupts enable (Port D)
	EduBase_Button_Interrupt_Init(&EduBase_Button_Handler);
	
	// Initialize the RGB LED (Port F)
	RGB_LED_Init();
	
	// Initialize Timer 0A to generate periodic interrupts every 1ms
	Timer_0A_Interrupt_Init(&Timer_0A_Periodic_Task);
	
	// Initialize a uint8_t array to store each digit of the stopwatch value
	uint8_t stopwatch_value[4] = {0};
	
	while(1)
	{
		Calculate_Stopwatch_Value(stopwatch_value);
		Seven_Segment_Display_Stopwatch(stopwatch_value); 
	}
}


/**
* @brief Handle the PMOD button press and performs the action to interrupt
*
* @param PMOD_BTN_Status of the PMOD buttons. Each button is represented differently
* 				0x04 for BTN0
*					0x08 for BTN1
*					0x10 for BTN2
*					0x20 for BTN3
*
* @return
*/
void PMOD_BTN_Handler(uint8_t pmod_btn_status)
{
	switch(pmod_btn_status)
	{
		// BTN0 (PA2) is pressed
		case 0x04:
		{
			RGB_LED_Output(RGB_LED_GREEN);
			start_stopwatch = 0x01;
			break;
		}
		
		// BTN1 (PA3) is pressed
		case 0x08:
		{
			RGB_LED_Output(RGB_LED_RED);
			start_stopwatch = 0x00;
			break;
		}
		
		// BTN2 (PA4) is pressed
		case 0x10:
		{
			RGB_LED_Output(RGB_LED_OFF);
			reset_stopwatch = 0x01;
			break;
		}
		
		//BTN3 (PA5) is pressed
		case 0x20:
		{
			//GPIOB->DATA = GPIOB->DATA ^ 0x08;
			break;
		}
		
		default:
		{
			break;
		}
	}
}

/**
* @brief The button will respond when being pressed on the EduBase board and adjust a counter based when pressed. 
*				
*				This function should increment by 1 and reset counter to zero if it is greater then or equal to 15.
*				Otherwise, the function should decrement counter by 1 and reset counter to zero if it leass than or equal to zero.
*
* @param	EduBase_button_status
*					-0x08: Increment the counter
*					-0x04: Decrement the counter
*
* @return
*/
void EduBase_Button_Handler(uint8_t edubase_button_status)
{
	switch(edubase_button_status)
	{
		case 0x08:
		{
			if (counter >= 15)
			{
				counter = 0;
			}
			else
			{
				counter = counter + 1;
			}
			break;
		}
		
		case 0x04:
		{
			if (counter <= 0)
			{
				counter = 15;
			}
			else
			{
				counter = counter - 1;
			}
			break;
		}
		
		default:
		{
			break;
		}
	}
}

/**
* @brief Calculates and stores the stopwatch time values into a provided array
*
*	This function extract the current stopwatch values. Each element of the array
* has specific stopwatch time:
* -Index 0: Milliseconds
* -Index 1: Least significant digit of seconds
* -Index 2: Most significant digit of seconds
* -Index 3: Minutes
*
* @param uint8_t stopwatch_value[]
*
* @return None
*/
void Calculate_Stopwatch_Value(uint8_t stopwatch_value[])
{
	// Store the "milliseconds" value in the first index of the array
	stopwatch_value[0] = milliseconds;
	
	// Store the least significant digit of the "seconds" value
	// in the second index of the array
	stopwatch_value[1] = seconds % 10;
	
	// Store the most significant digit of the "seconds" value
	// in the third index of the array
	stopwatch_value[2] = seconds / 10;
	
	// Store the "minutes" value in the fourth index of the array
	stopwatch_value[3] = minutes;
}

/**
* @brief The Periodic task will manage the stopwatch's time progression.
*
*	It increments the millisecond, second, and minute values of stopwatch
* when the start stopwatch flag is set. The stopwatch will reset when the
* the reset stopwatch flag is set. 
* - Milliseconds increment after every 100ms
* - Seconds increment after 10 milliseconds
* - Minutes increment after every 60 seconds
*
* @param None
*
* @return None
*/
void Timer_0A_Periodic_Task(void)
{
	if (start_stopwatch == 0x01)
	{
		ms_elapsed++;
		if(ms_elapsed > 99)
		{
			ms_elapsed = 0;
			milliseconds++;
		}
		
		if(milliseconds > 9)
		{
			milliseconds = 0;
			seconds++;
		}
		
		if (seconds > 59)
		{
			seconds = 0;
			minutes++;
		}
		
		if (minutes > 9)
		{
			minutes = 0;
			
		}
		
		if (reset_stopwatch == 0x01)
		{
			reset_stopwatch = 0x00;
			start_stopwatch = 0x00;
			ms_elapsed = 0;
			milliseconds = 0;
			seconds = 0;
			minutes = 0;
		}
	}
}