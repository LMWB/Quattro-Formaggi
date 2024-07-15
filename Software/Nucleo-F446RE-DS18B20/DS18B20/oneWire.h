/*
 * oneWire.h
 *
 *  Created on: Jan 15, 2024
 *      Author: lukas
 */

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

/* mostly take from MAXIM / Analog Devices data-sheet and application-notes */


/* ################ Platform Specific ####################### */
#include "usart.h"
#include "tim.h"

#define ONEWIRE_UART              huart2
#define TOOLS_HANDLER_UART        ONEWIRE_UART
#define ONEWIRE_TIMER             htim6

/* get/set OW_RX aliases */
//#define OW_RX_GetValue()  HAL_GPIO_ReadPin(OneWireRX_alt_GPIO_Port, OneWireRX_alt_Pin)
#define OW_RX_GetValue()  HAL_GPIO_ReadPin(OneWireRX_GPIO_Port, OneWireRX_Pin)

/* get/set OW_TX aliases */
//#define OW_TX_SetHigh()   HAL_GPIO_WritePin(OneWireTX_alt_GPIO_Port, OneWireTX_alt_Pin, 1)
//#define OW_TX_SetLow()    HAL_GPIO_WritePin(OneWireTX_alt_GPIO_Port, OneWireTX_alt_Pin, 0)
#define OW_TX_SetHigh()   HAL_GPIO_WritePin(OneWireTX_GPIO_Port, OneWireTX_Pin, 1)
#define OW_TX_SetLow()    HAL_GPIO_WritePin(OneWireTX_GPIO_Port, OneWireTX_Pin, 0)

/*
 * TIM6 clock = 84MHz
 * TIM6 PSC = 21
 * TIM6 tick = 21/84MHz = 1/4us = 0.25us
 * Pause for exactly 'tick' number of ticks = 0.25us void tickDelay(int tick); // Implementation is platform specific
 */
#define OW_TICK_DELAY 4

/* start timer 6 for executing measurement */
#define OW_START_TIMER()      do{ HAL_TIM_Base_Start(&ONEWIRE_TIMER); } while(0)
/* reset hardware timer timer value to 0 */
#define OW_RESET_TIMER()      __HAL_TIM_SET_COUNTER(&ONEWIRE_TIMER, 0)
/* stop timer and reset its value */
#define OW_STOP_TIMER()       do{HAL_TIM_Base_Stop(&ONEWIRE_TIMER); OW_RESET_TIMER();} while(0)
/* return timer current value */
#define OW_GET_TIMER_VALUE()  __HAL_TIM_GET_COUNTER(&ONEWIRE_TIMER)


/* ################ One Wire Protocol Driver ####################### */
/* Public Variables */
/* temporary storage for one wire search algorithm */
extern unsigned char OWROM_NO[8];

typedef enum __OWDevcieFamilyCode{
  owFamilyCodeDS18B20 = 0x28,

}OWDevcieFamilyCode_t;

/* Public Functions */
void OWSetSpeed( uint8_t standard );

uint8_t OWTouchReset( void );

void OWWriteBit( uint8_t bit );

uint8_t OWReadBit( void );

void OWWriteByte( uint8_t data );

uint8_t OWReadByte( void );

/*
 * The owTouchByte operation is a simultaneous write and read from the 1-Wire bus. This function was
 * derived so that a block of both writes and reads could be constructed. This is more efficient on some
 * platforms and is commonly used in API's provided by Maxim. The OWBlock function simply sends and
 * receives a block of data to the 1-Wire using the OWTouchByte function. Note that OWTouchByte(0xFF)
 * is equivalent to OWReadByte() and OWTouchByte(data) is equivalent to OWWriteByte(data)
 */
int OWTouchByte( int data );

void OWBlock( unsigned char *data, int data_len );

uint8_t OWCRC8( uint8_t *addr, uint8_t len );

int OWFirst( );

int OWNext( );

int OWSearch( );


#endif /* ONEWIRE_H_ */
