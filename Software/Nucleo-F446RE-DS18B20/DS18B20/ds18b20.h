/*
 * ds18b20.h
 *
 *  Created on: Jan 12, 2024
 *      Author: lukas
 */

#ifndef DS18B20_H_
#define DS18B20_H_

#include "oneWire.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> //for va_list var arg function


/* User Defines */
#define DS18B20_MAX_DEVICES_ON_BUS 32

/* Library Defines */
typedef enum {
  DS18B20_RESOLUTION_9Bits = 0x00, /* dT= 0.5C      max conversion time  93.75ms = t_conv/8 */
  DS18B20_RESOLUTION_10Bits = 0x01, /* dT= 0.25C     max conversion time 187.50ms = t_conv/4 */
  DS18B20_RESOLUTION_11Bits = 0x02, /* dT= 0.125C    max conversion time 375.00ms = t_conv/2 */
  DS18B20_RESOLUTION_12Bits = 0x03, /* dT= 0.0.0625C max conversion time 750.00ms = t_conv   */
} DS18B20_RESOLUTION_t;

typedef enum {
  DS18B20_ROM_COMMAND_Read = 0x33,
  DS18B20_ROM_COMMAND_Match = 0x55,
  DS18B20_ROM_COMMAND_Search = 0xf0,
  DS18B20_ROM_COMMAND_Skip = 0xcc,
  DS18B20_ROM_COMMAND_Alarm = 0xec,
} DS18B20_ROM_COMMANDS_t;

typedef enum {
  DS18B20_FUCTION_COMMAND_Convert_T = 0x44,
  DS18B20_FUCTION_COMMAND_WriteScratchpad = 0x4e,
  DS18B20_FUCTION_COMMAND_ReadScratchpad = 0xbe,
  DS18B20_FUCTION_COMMAND_CopyScratchpad = 0x48,
  DS18B20_FUCTION_COMMAND_RecallEE = 0xb8,
  DS18B20_FUCTION_COMMAND_ReadPowerSupply = 0xb4,
} DS18B20_FUCTION_COMMANDS_t;

typedef enum {
  DS18B20_STATUS_NO_PRESENCE_PULSE = 0u, DS18B20_STATUS_RESPONDING = 1u,
} DS18B20_STATUS_t;

/* Handler / Class */

typedef struct _DS18B20_ID{
  uint8_t familycode;
  uint8_t ID[6];
  uint8_t CRC_Byte;
}_DS18B20_ID_t;

typedef struct {
  /* in case there are more than one sensors on the same bus this is necessary */
  uint8_t familycode;
  uint8_t ID[6];
  uint8_t CRC_Byte;

  /* non volatile programmed resolution of each sensor, 12Bit = default */
  uint8_t resolution;

  /* scratchpad buffer */
  uint8_t temperature_buffer[2]; /* reading MSB and LSB Byte from ds18b20 scratchpad */
  int16_t temperature_raw; /* combining MSB and LSB from scratchpad */
  int32_t temperature; /* converting raw value to grad celsius with Factor x1000 */

} DS18B20_t;

/* DS18B20 temperature One Wire Sensor*/
extern DS18B20_t sensor_1;

void ds18b20_demo( void );

void ds18b20_demo_multi( void );

/* todo
 * in case of multiple (unknown) DS18B20 devices on the bus
 * one has to run a teaching phase to let the system recognize which sensor is
 * connected on which terminal
 *
 * e.g. avoid reading sensor on terminal 1 (room temperature)
 * but user has put outside temperature on that slot
 * */
void ds18b20_demo_multi_teach( void );

/*
 * This function makes shure that we access a DS18B20 Temperature Sensor
 * check one specific device in ALL_DEVICESS memory for DS18B20 Family Code
 *
 */
uint8_t ds18b20_check_device_familiy_code( DS18B20_t *sensor );

/*
 * The master device must issue an appropriate ROM command before
 * issuing a DS18B20 function command.
 *
 * For example, the master can make all
 * DS18B20s on the bus perform simultaneous temperature
 * conversions by issuing a Skip ROM command followed by
 * a Convert T [44h] command.
 *
 * Note that the Read Scratchpad [BEh] command can
 * follow the Skip ROM command only if there is a single
 * slave device on the bus. In this case, time is saved by
 * allowing the master to read from the slave without send-
 * ing the device?s 64-bit ROM code. A Skip ROM command
 * followed by a Read Scratchpad command will cause
 * a data collision on the bus if there is more than one
 * slave since multiple devices will attempt to transmit data
 * simultaneously.
 */
uint8_t ds18b20_read_temperature_single( DS18B20_t *sensor );

/* todo
 * same as above but with a known 1-wire DS18B20 Serial Number */
uint8_t ds18b20_read_temperature_single_ID( DS18B20_t *sensor, _DS18B20_ID_t id );

/* */
uint8_t ds18b20_read_signature_single( DS18B20_t *sensor );

/*  todo */
uint8_t ds18b20_read_temperature_multi( );

/*
 * This command can only be used when there is one slave
 * on the bus. It allows the bus master to read the slave?s
 * 64-bit ROM code without using the Search ROM proce-
 * dure. If this command is used when there is more than
 * one slave present on the bus, a data collision will occur
 * when all the slaves attempt to respond at the same time.
 */
uint8_t ds18b20_read_signature_multi( );


/*todo move to Application */
void my_printf( const char *fmt, ... );

#endif /* DS18B20_H_ */
