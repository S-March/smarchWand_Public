/**
 ****************************************************************************************
 *
 * @file user_barebone.h
 *
 * @brief Barebone application header file.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef _USER_BAREBONE_H_
#define _USER_BAREBONE_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup
 *
 * @brief
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include "gapc_task.h"                 // gap functions and messages
#include "app_task.h"                  // application task
#include "app.h"                       // application definitions
#include "app_callback.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/* Duration of timer for connection parameter update request */
#define APP_PARAM_UPDATE_REQUEST_TO         (1000)   // 1000*10ms = 10sec, The maximum allowed value is 41943sec (4194300 * 10ms)

/* Advertising data update timer */
//#define APP_ADV_DATA_UPDATE_TO              (3000)   // 3000*10ms = 30sec, The maximum allowed value is 41943sec (4194300 * 10ms)
#define APP_ADV_DATA_UPDATE_TO              (1)   // 1*10ms = 0.01sec, The maximum allowed value is 41943sec (4194300 * 10ms)
#define APP_GESTURE_LOCK_TO              		(50)   // 50*10ms = 0.5sec, The maximum allowed value is 41943sec (4194300 * 10ms)
#define APP_GESTURE_RESET_DISPLAY_TO       	(300)   // 300*10ms = 3sec, The maximum allowed value is 41943sec (4194300 * 10ms)
#define APP_MOTOR_ON_TO       							(100)   // 50*10ms = 0.5sec, The maximum allowed value is 41943sec (4194300 * 10ms)

#define XL_AVERAGE_NUM_MULTIPLIER						3
#define XL_AVERAGE_NUM_SAMPLES							(16) //8

/* Manufacturer specific data constants */
//#define APP_AD_MSD_COMPANY_ID               (0xABCD)
#define APP_AD_MSD_COMPANY_ID               (0x00)//(0x06190E)
#define APP_AD_MSD_COMPANY_ID_LEN           (1)//(3)
#define APP_AD_MSD_DATA_NUM_BYTES           (5)
#define APP_AD_MSD_DATA_LEN                 (APP_AD_MSD_DATA_NUM_BYTES*sizeof(uint8_t))

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Application initialization function.
 ****************************************************************************************
*/
void user_app_init(void);

/**
 ****************************************************************************************
 * @brief Advertising function.
 ****************************************************************************************
*/
void user_app_adv_start(void);

/**
 ****************************************************************************************
 * @brief Connection function.
 * @param[in] conidx        Connection Id index
 * @param[in] param         Pointer to GAPC_CONNECTION_REQ_IND message
 ****************************************************************************************
*/
void user_app_connection(const uint8_t conidx, struct gapc_connection_req_ind const *param);

/**
 ****************************************************************************************
 * @brief Undirect advertising completion function.
 * @param[in] status Command complete event message status
 ****************************************************************************************
*/
void user_app_adv_undirect_complete(uint8_t status);

/**
 ****************************************************************************************
 * @brief Disconnection function.
 * @param[in] param         Pointer to GAPC_DISCONNECT_IND message
 ****************************************************************************************
*/
void user_app_disconnect(struct gapc_disconnect_ind const *param);

/**
 ****************************************************************************************
 * @brief Handles the messages that are not handled by the SDK internal mechanisms.
 * @param[in] msgid   Id of the message received.
 * @param[in] param   Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id  ID of the sending task instance.
 ****************************************************************************************
*/
void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id);
/**
 ****************************************************************************************
 * @brief Waking up from sleep
 ****************************************************************************************
*/
void user_app_waking_from_sleep(void);
/**
 ****************************************************************************************
 * @brief Going tosleep
 ****************************************************************************************
*/
void user_app_going_to_sleep(void);

/// @} APP

#endif //_USER_BAREBONE_H_
