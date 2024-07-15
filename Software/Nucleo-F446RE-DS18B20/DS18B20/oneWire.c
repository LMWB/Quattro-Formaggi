/*
 * oneWire.c
 *
 *  Created on: Jan 15, 2024
 *      Author: lukas
 */

#include "oneWire.h"
#include <stdbool.h>
#include <stdio.h>



/*
 * TIM6 clock = 84MHz
 * TIM6 PSC = 21
 * TIM6 tick = 21/84MHz = 1/4us = 0.25us
 */
static void delay_us( uint16_t microseconds ) {
  uint16_t delay = microseconds;
  OW_START_TIMER( );
  while ( OW_GET_TIMER_VALUE() < delay ) {
    ;
  }
  OW_STOP_TIMER( );
}

/*
 * Pause for exactly 'tick' number of ticks = 0.25us
 * Implementation is platform specific
 */
static void tickDelay( int tick ) {
  delay_us( (uint16_t) tick );
}




/* Public Variables */
unsigned char OWROM_NO[8] = {0};

/* Private Variables */
static int s_LastDiscrepancy;
static int s_LastFamilyDiscrepancy;
static int s_LastDeviceFlag;
static unsigned char s_CRC8;


/* DEPRECATED
 * save one bit out of the 64Bit Serial No */
static void ow_search_save_ROM( uint8_t *ROM, uint8_t bit_pos, uint8_t bit_value ) {
  if ( bit_pos < 8 ) {
    ROM[0] |= bit_value << (bit_pos);
  }

  else if ( bit_pos < 16 ) {
    ROM[1] |= bit_value << (bit_pos - 8);
  }

  else if ( bit_pos < 24 ) {
    ROM[2] |= bit_value << (bit_pos - 16);
  }

  else if ( bit_pos < 32 ) {
    ROM[3] |= bit_value << (bit_pos - 24);
  }

  else if ( bit_pos < 40 ) {
    ROM[4] |= bit_value << (bit_pos - 32);
  }

  else if ( bit_pos < 48 ) {
    ROM[5] |= bit_value << (bit_pos - 40);
  }

  else if ( bit_pos < 56 ) {
    ROM[6] |= bit_value << (bit_pos - 48);
  }

  else if ( bit_pos < 64 ) {
    ROM[7] |= bit_value << (bit_pos - 56);
  } else {
    ;
  }
}


// 'tick' values
static int A, B, C, D, E, F, G, H, I, J;

/*
 * Set the 1-Wire timing to 'standard' (standard=1) or 'overdrive'(standard=0).
 */
void OWSetSpeed( uint8_t standard ) {
// Adjust tick values depending on speed
  if ( standard ) {
    // Standard Speed
    A = 6 * OW_TICK_DELAY;
    B = 64 * OW_TICK_DELAY;
    C = 60 * OW_TICK_DELAY;
    D = 10 * OW_TICK_DELAY;
    E = 9 * OW_TICK_DELAY;
    F = 55 * OW_TICK_DELAY;
    G = 0;
    H = 480 * OW_TICK_DELAY;
    I = 70 * OW_TICK_DELAY;
    J = 410 * OW_TICK_DELAY;
  } else {
    // Overdrive Speed
    A = 1.5 * OW_TICK_DELAY;
    B = 7.5 * OW_TICK_DELAY;
    C = 7.5 * OW_TICK_DELAY;
    D = 2.5 * OW_TICK_DELAY;
    E = 0.75 * OW_TICK_DELAY;
    F = 7 * OW_TICK_DELAY;
    G = 2.5 * OW_TICK_DELAY;
    H = 70 * OW_TICK_DELAY;
    I = 8.5 * OW_TICK_DELAY;
    J = 40 * OW_TICK_DELAY;
  }
}

// Generate a 1-Wire reset, return 0 if no presence detect was found,
// return 1 otherwise.
// (NOTE: Does not handle alarm presence from DS2404/DS1994)
//
uint8_t OWTouchReset( void ) {
  uint8_t result = 0;
  tickDelay( G );
  OW_TX_SetLow( );              // Drives DQ low
  tickDelay( H );
  OW_TX_SetHigh( );             // Releases the bus
  tickDelay( I );
  result = ~OW_RX_GetValue( );  // Sample for presence pulse from slave
  tickDelay( J );               // Complete the reset sequence recovery
  return result;                // Return sample presence pulse result
}

// Send a 1-Wire write bit. Provide 10us recovery time.
//

void OWWriteBit( uint8_t bit ) {
  if ( bit ) {
    // Write '1' bit
    OW_TX_SetLow( ); // Drives DQ low
    tickDelay( A );
    OW_TX_SetHigh( ); // Releases the bus
    tickDelay( B ); // Complete the time slot and 10us recovery
  } else {
    // Write '0' bit
    OW_TX_SetLow( ); // Drives DQ low
    tickDelay( C );
    OW_TX_SetHigh( ); // Releases the bus
    tickDelay( D );
  }
}

// Read a bit from the 1-Wire bus and return it. Provide 10us recovery time.
//
uint8_t OWReadBit( void ) {
  uint8_t result;
  OW_TX_SetLow( ); // Drives DQ low
  tickDelay( A );
  OW_TX_SetHigh( ); // Releases the bus
  tickDelay( E );
  result = OW_RX_GetValue( ); // Sample the bit value from the slave
  tickDelay( F ); // Complete the time slot and 10us recovery
  return result;
}

// Write 1-Wire data byte
//

void OWWriteByte( uint8_t data ) {
  int loop;
// Loop to write each bit in the byte, LS-bit first
  for ( loop = 0; loop < 8; loop++ ) {
    OWWriteBit( data & 0x01 );
    // shift the data byte for the next bit
    data >>= 1;
  }
}

// Read 1-Wire data byte and return it
//
uint8_t OWReadByte( void ) {
  uint8_t loop, result = 0;
  for ( loop = 0; loop < 8; loop++ ) {
    // shift the result to get it ready for the next bit
    result >>= 1;
    // if result is one, then set MS bit
    if ( OWReadBit( ) )
      result |= 0x80;
  }
  return result;
}

// Write a 1-Wire data byte and return the sampled result.
//
int OWTouchByte( int data ) {
  int loop, result = 0;
  for ( loop = 0; loop < 8; loop++ ) {
    // shift the result to get it ready for the next bit
    result >>= 1;
    // If sending a '1' then read a bit else write a '0'
    if ( data & 0x01 ) {
      if ( OWReadBit( ) )
        result |= 0x80;
    } else
      OWWriteBit( 0 );
    // shift the data byte for the next bit
    data >>= 1;
  }
  return result;
}

// Write a block 1-Wire data bytes and return the sampled result in the same
// buffer.
//
void OWBlock( unsigned char *data, int data_len ) {
  int loop;
  for ( loop = 0; loop < data_len; loop++ ) {
    data[loop] = OWTouchByte( data[loop] );
  }
}

uint8_t OWCRC8( uint8_t *addr, uint8_t len ) {
  uint8_t crc = 0, inbyte, i, mix;

  while ( len-- ) {
    inbyte = *addr++;
    for ( i = 8; i; i-- ) {
      mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if ( mix ) {
        crc ^= 0x8C;
      }
      inbyte >>= 1;
    }
  }

  /* Return calculated CRC */
  return crc;
}

// Find the 'first' devices on the 1-Wire bus
// Return TRUE : device found, ROM number in ROM_NO buffer
// FALSE : no device present
//
int OWFirst( ) {
// reset the search state
  s_LastDiscrepancy = 0;
  s_LastDeviceFlag = false;
  s_LastFamilyDiscrepancy = 0;
  return OWSearch( );
}

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire bus
// Return TRUE : device found, ROM number in ROM_NO buffer
// FALSE : device not found, end of search
//
int OWNext( ) {
// leave the search state alone
  return OWSearch( );
}

//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return TRUE : device found, ROM number in ROM_NO buffer
// FALSE : device not found, end of search
//
int OWSearch( ) {
  int id_bit_number;
  int last_zero, rom_byte_number, search_result;
  int id_bit, cmp_id_bit;
  unsigned char rom_byte_mask, search_direction;

  // initialize for search
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;
  s_CRC8 = 0;

  // if the last call was not the last one
  if ( !s_LastDeviceFlag ) {

    // 1-Wire reset
    if ( !OWTouchReset( ) ) {

      // reset the search
      s_LastDiscrepancy = 0;
      s_LastDeviceFlag = false;
      s_LastFamilyDiscrepancy = 0;
      return false;
    }

    // issue the search command
    OWWriteByte( 0xF0 );

    // loop to do the search
    do {

      // read a bit and its complement
      id_bit = OWReadBit( );
      cmp_id_bit = OWReadBit( );

      // check for no devices on 1-wire
      if ( (id_bit == 1) && (cmp_id_bit == 1) )
        break;
      else {

        // all devices coupled have 0 or 1
        if ( id_bit != cmp_id_bit )
          search_direction = id_bit; // bit write value for search
        else {

          // if this discrepancy if before the Last Discrepancy
          // on a previous next then pick the same as last time
          if ( id_bit_number < s_LastDiscrepancy )
            search_direction = ((OWROM_NO[rom_byte_number] & rom_byte_mask) > 0);
          else

            // if equal to last pick 1, if not then pick 0
            search_direction = (id_bit_number == s_LastDiscrepancy);

          // if 0 was picked then record its position in LastZero
          if ( search_direction == 0 ) {
            last_zero = id_bit_number;
            // check for Last discrepancy in family
            if ( last_zero < 9 )
              s_LastFamilyDiscrepancy = last_zero;
          }
        }
        // set or clear the bit in the ROM byte rom_byte_number
        // with mask rom_byte_mask
        if ( search_direction == 1 )
          OWROM_NO[rom_byte_number] |= rom_byte_mask;
        else
          OWROM_NO[rom_byte_number] &= ~rom_byte_mask;
        // serial number search direction write bit
        OWWriteBit( search_direction );
        // increment the byte counter id_bit_number
        // and shift the mask rom_byte_mask
        id_bit_number++;
        rom_byte_mask <<= 1;
        // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
        if ( rom_byte_mask == 0 ) {
          // todo crc-8 implementation
          //docrc8( ROM_NO[rom_byte_number] ); // accumulate the CRC
          rom_byte_number++;
          rom_byte_mask = 1;
        }
      }
    } while ( rom_byte_number < 8 ); // loop until through all ROM bytes 0-7
    // if the search was successful then
    if ( !((id_bit_number < 65) || (s_CRC8 != 0)) ) {
      // search successful so set LastDiscrepancy, s_LastDeviceFlag, search_result
      s_LastDiscrepancy = last_zero;
      // check for last device
      if ( s_LastDiscrepancy == 0 )
        s_LastDeviceFlag = true;
      search_result = true;
    }
  }

  // if no device found then reset counters so next 'search' will be like a first
  if ( !search_result || !OWROM_NO[0] ) {
    s_LastDiscrepancy = 0;
    s_LastDeviceFlag = false;
    s_LastFamilyDiscrepancy = 0;
    search_result = false;
  }
  return search_result;
}
