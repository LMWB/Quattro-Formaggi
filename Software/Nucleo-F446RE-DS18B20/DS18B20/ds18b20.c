/*
 * ds18b20.c
 *
 *  Created on: Jan 12, 2024
 *      Author: lukas
 */

#include "ds18b20.h"

#include <stdbool.h>

/* Public / Exported Variables */
DS18B20_t sensor_1 = { 0 };

/* Private Variables */
static uint8_t NO_OF_DEVICES_ON_BUS = 0;
static unsigned char ALL_DEVICESS[DS18B20_MAX_DEVICES_ON_BUS][8]; // maximum allowed DS18B20 1Wire devices on the bus

/* Public / Exported Functions */
void ds18b20_demo( void ) {
  uint8_t dummy = 0;
  uint8_t result = 0; /* 0=fail, 1=good */

  OWSetSpeed( 1 );
  while ( 1 ) {

    (void) ds18b20_read_signature_single( &sensor_1 );
    (void) ds18b20_read_temperature_single( &sensor_1 );

    (void) my_printf( "DS18B20 MSB; %x; LSB; %x; \n", sensor_1.temperature_buffer[1],
        sensor_1.temperature_buffer[0] );
    (void) my_printf( "DS18B20 Temp Raw; %d; \n", sensor_1.temperature_raw );
    (void) my_printf( "DS18B20 Temp; %d; \n", sensor_1.temperature );

    (void) my_printf( "DS18B20 Family Code %d; \n", sensor_1.familycode );
    (void) my_printf( "DS18B20 CRC Byte %d; \n", sensor_1.CRC_Byte );
    (void) my_printf( "DS18B20 Ser.No [MSB...LSB]: %d %d %d %d %d %d; \n", sensor_1.ID[5],
        sensor_1.ID[4], sensor_1.ID[3], sensor_1.ID[2], sensor_1.ID[1], sensor_1.ID[0] );
    (void) my_printf( "------------------------------------ \n\n", sensor_1.familycode );
  }
  dummy = dummy;
  result = result;
}

void ds18b20_demo_multi_teach( void )
{
  ;;;
}

void ds18b20_demo_multi( void ) {
  uint8_t dummy = 0;
  uint8_t result = 0; /* 0=fail, 1=good */

  OWSetSpeed( 1 );
  my_printf( "\nStart 1-Wire Search Algorithm ... \n" );
  bool rslt = OWFirst( );
  uint8_t n = 0;
  while ( rslt && (n < DS18B20_MAX_DEVICES_ON_BUS) ) {
    // save this device on static variable
    memcpy( ALL_DEVICESS[n], OWROM_NO, sizeof(OWROM_NO) );

    // print device found
    my_printf( "..Found Device with SerNo: %d %d %d %d %d %d %d %d\n", ALL_DEVICESS[n][7],
        ALL_DEVICESS[n][6], ALL_DEVICESS[n][5], ALL_DEVICESS[n][4], ALL_DEVICESS[n][3],
        ALL_DEVICESS[n][2], ALL_DEVICESS[n][1], ALL_DEVICESS[n][0] );

    // search next device on that may be present on bus
    n++;
    rslt = OWNext( );
  }
  my_printf( "Summary: Found %d Devices on the 1-Wire Bus\n", n );
  NO_OF_DEVICES_ON_BUS = n;

  my_printf( "Start Continuously Reading of DS18B20 Sensors on the 1-Wire Bus\n\n" );
  while ( 1 ) {
    /*
     * The master can use this command to address all devices
     * on the bus simultaneously without sending out any ROM
     * code information. For example, the master can make all
     * DS18B20s on the bus perform simultaneous temperature
     * conversions by issuing a Skip ROM command followed by
     * a Convert T [44h] command
     */

    if ( OWTouchReset( ) ) {
      OWWriteByte( DS18B20_ROM_COMMAND_Skip );
      OWWriteByte( DS18B20_FUCTION_COMMAND_Convert_T );

      /* polling for conversion complete aprox. 750ms with 12Bit resolution
       * with multiple devices on the bus I don't know how is answering first
       * */
      do {
        dummy = OWReadBit( );
      } while ( !dummy );
      result = 1;
    }

    // read all devices temperature
    /* Read Scratchpad upper 2 Bytes (temperature Byte0 LSB & Byte1 MSB) */
    /*
     * The match ROM command followed by a 64-bit ROM
     * code sequence allows the bus master to address a
     * specific slave device on a multidrop or single-drop bus.
     * Only the slave that exactly matches the 64-bit ROM code
     * sequence will respond to the function command issued
     * by the master; all other slaves on the bus will wait for a
     * reset pulse.
     */
    if ( result ) {
      result = 0;

      /** loop through local buffer **/
      for ( uint_fast8_t i = 0; i < NO_OF_DEVICES_ON_BUS; i++ ) {

        /* reset bus */
        if ( OWTouchReset( ) ) {

          /* fetch a device from local 1-wire buffer */
          sensor_1.familycode = ALL_DEVICESS[i][0];
          sensor_1.ID[0] = ALL_DEVICESS[i][1];
          sensor_1.ID[1] = ALL_DEVICESS[i][2];
          sensor_1.ID[2] = ALL_DEVICESS[i][3];
          sensor_1.ID[3] = ALL_DEVICESS[i][4];
          sensor_1.ID[4] = ALL_DEVICESS[i][5];
          sensor_1.ID[5] = ALL_DEVICESS[i][6];
          sensor_1.CRC_Byte = ALL_DEVICESS[i][7];

          /* check that scanned devices are of type DS18B20,
           * of not reading the temperature register
           * (so called scratchpad) would not make any sense */
          if ( ds18b20_check_device_familiy_code( &sensor_1 ) ) {
            OWWriteByte( DS18B20_ROM_COMMAND_Match );
            OWWriteByte( sensor_1.familycode );
            OWWriteByte( sensor_1.ID[0] );
            OWWriteByte( sensor_1.ID[1] );
            OWWriteByte( sensor_1.ID[2] );
            OWWriteByte( sensor_1.ID[3] );
            OWWriteByte( sensor_1.ID[4] );
            OWWriteByte( sensor_1.ID[5] );
            OWWriteByte( sensor_1.CRC_Byte );
            OWWriteByte( DS18B20_FUCTION_COMMAND_ReadScratchpad );

            /* read scratchpad */
            sensor_1.temperature_buffer[0] = OWReadByte( );
            sensor_1.temperature_buffer[1] = OWReadByte( );
            /* convert raw binary to decrease celsius */
            sensor_1.temperature_raw = (sensor_1.temperature_buffer[1] << 8)
                | sensor_1.temperature_buffer[0];
            sensor_1.temperature = sensor_1.temperature_raw * 1000L / 16L;
            result = 1;
          }
        }

        if ( result ) {
          my_printf( "DS18B20 No %i has Temp of %d \n", (i + 1), sensor_1.temperature );
        } else {
          my_printf( "Device not of type DS18B20 \n" );
        }

        /** end of looping through local buffer **/
      }
    }

    my_printf( "\n" );
    HAL_GPIO_TogglePin( LD2_GPIO_Port, LD2_Pin );
    HAL_Delay( 300 );
  }
}

uint8_t ds18b20_check_device_familiy_code( DS18B20_t *sensor ) {
  uint8_t result = 0;

  if ( sensor->familycode == owFamilyCodeDS18B20 ) {
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

uint8_t ds18b20_read_temperature_single( DS18B20_t *sensor ) {
  uint8_t dummy = 0;
  uint8_t result = 0; /* 0=fail, 1=good */

  if ( OWTouchReset( ) ) {

    OWWriteByte( DS18B20_ROM_COMMAND_Skip );
    OWWriteByte( DS18B20_FUCTION_COMMAND_Convert_T );

    /* polling for conversion complete aprox. 750ms with 12Bit resolution */
    do {
      dummy = OWReadBit( );
    } while ( !dummy );
    result = 1;

  } else {
    /* no ds18b20 sensor on responding on the bus*/
    result = 0;
  }

  /* Read Scratchpad upper 2 Bytes (temperature Byte0 LSB & Byte1 MSB) */
  if ( OWTouchReset( ) ) {
    OWWriteByte( DS18B20_ROM_COMMAND_Skip );
    OWWriteByte( DS18B20_FUCTION_COMMAND_ReadScratchpad );

    sensor->temperature_buffer[0] = OWReadByte( );
    sensor->temperature_buffer[1] = OWReadByte( );
    sensor->temperature_raw = (sensor->temperature_buffer[1] << 8) | sensor->temperature_buffer[0];
    sensor->temperature = sensor->temperature_raw * 1000L / 16L;

    result = 1;
  } else {
    /* no ds18b20 sensor on responding on the bus*/
    result = 0;
  }

  return result;
}

uint8_t ds18b20_read_temperature_multi( )
{
  return 0;
}

uint8_t ds18b20_read_signature_single( DS18B20_t *sensor ) {
  uint8_t dummy[8];
  uint8_t loop;
  uint8_t result = DS18B20_STATUS_NO_PRESENCE_PULSE; /* 0=fail, 1=good */

  if ( OWTouchReset( ) ) {
    OWWriteByte( DS18B20_ROM_COMMAND_Read );

    for ( loop = 0; loop < 8; loop++ ) {
      dummy[loop] = OWReadByte( );
    }
    sensor->familycode = dummy[0];

    sensor->ID[0] = dummy[1];
    sensor->ID[1] = dummy[2];
    sensor->ID[2] = dummy[3];
    sensor->ID[3] = dummy[4];
    sensor->ID[4] = dummy[5];
    sensor->ID[5] = dummy[6];

    sensor->CRC_Byte = dummy[7];

    result = DS18B20_STATUS_RESPONDING;
  } else {
    result = DS18B20_STATUS_NO_PRESENCE_PULSE;
  }
  return result;
}

void my_printf( const char *fmt, ... ) {
  static char buffer[256];
  va_list args;
  va_start( args, fmt );
  vsnprintf( buffer, sizeof(buffer), fmt, args );
  va_end( args );

  int len = strlen( buffer );
  HAL_UART_Transmit( &TOOLS_HANDLER_UART, (uint8_t*) buffer, len, -1 );
}
