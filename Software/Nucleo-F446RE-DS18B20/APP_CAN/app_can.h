/*
 * app_can.h
 *
 *  Created on: Feb 15, 2024
 *      Author: lukas
 */

#ifndef APP_CAN_H_
#define APP_CAN_H_

#include <stdint.h>

void app_can_set_msg_filter(void);

void app_can_start(void);

uint8_t app_can_send_test_msg(void);


#endif /* APP_CAN_H_ */
