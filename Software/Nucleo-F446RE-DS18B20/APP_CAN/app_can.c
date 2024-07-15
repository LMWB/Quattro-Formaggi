/*
 * app_can.c
 *
 *  Created on: Feb 15, 2024
 *      Author: lukas
 */

#include "app_can.h"
#include "can.h"

static CAN_FilterTypeDef sFilterConfig;
static uint8_t sTxTestMsg[8] = {'A', 'R', 'D', 'U', 'I', 'N', 'O', '!'};
static CAN_TxHeaderTypeDef sTxHeaderTestMsg;
static uint32_t TxMailbox_0 = 0;

void app_can_set_msg_filter( void ) {
    // nothing to be filtered in default mode
    // all can messages are allowed to pass

    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

    sFilterConfig.FilterIdHigh = 0x00;
    sFilterConfig.FilterIdLow = 0x00;
    sFilterConfig.FilterMaskIdHigh = 0x0;
    sFilterConfig.FilterMaskIdLow = 0x0;

    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.FilterBank = 0;

    HAL_CAN_ConfigFilter( &hcan2, &sFilterConfig );
    HAL_CAN_ActivateNotification( &hcan2, CAN_IT_RX_FIFO0_MSG_PENDING );
    HAL_CAN_ActivateNotification( &hcan2, CAN_IT_RX_FIFO0_FULL );
    HAL_CAN_ActivateNotification( &hcan2, CAN_IT_RX_FIFO0_OVERRUN );
}

void app_can_start( void ) {
  HAL_CAN_Start( &hcan2 );
}

uint8_t app_can_send_test_msg(void){

    uint8_t err = 0;
    HAL_StatusTypeDef can_TX_RX_status;

    sTxHeaderTestMsg.StdId = 0xAA;
    sTxHeaderTestMsg.ExtId = 0xAABBCC;
    sTxHeaderTestMsg.RTR = CAN_RTR_DATA;
    sTxHeaderTestMsg.IDE = CAN_ID_EXT;
    sTxHeaderTestMsg.DLC = 8;
    sTxHeaderTestMsg.TransmitGlobalTime = DISABLE;
    can_TX_RX_status = HAL_CAN_AddTxMessage(&hcan2, &sTxHeaderTestMsg, &sTxTestMsg[0], &TxMailbox_0);
    if( HAL_OK != can_TX_RX_status )
    {
      err = 1;
    }

    return err;
}

