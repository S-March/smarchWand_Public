/**
 ****************************************************************************************
 *
 * @file user_barebone.c
 *
 * @brief Barebone project source code.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration
#include "gap.h"
#include "app_easy_timer.h"
#include "user_barebone.h"
#include "co_bt.h"

#include "systick.h" 
#include "uart_utils.h"
#include "user_periph_setup.h"
#include "arch_system.h"
#include "user_xl_driver.h"
#include "arch_console.h"
#include "math.h"
#include "stdlib.h"
#include "user_wakeup.h"
#include "wkupct_quadec.h" 


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/****************************************************************************************/
/* User constants                                                                   		  	  				*/
/****************************************************************************************/
const uint32_t BLINK_INTERVAL	= 500000;	// In usec.
const uint32_t LONG_PRESS_TIME	= 3000000;	// In usec.
const uint8_t DEBOUNCE_MS	= 30;	// In ms.
const bool INPUT_LEVEL = true;	// Input will generate IRQ if input is low.
const bool EDGE_LEVEL = true;	// Wait for key release after interrupt was set.

uint8_t xlDataArrayCounter;
volatile int8_t xDataArray[XL_AVERAGE_NUM_SAMPLES];
volatile int8_t yDataArray[XL_AVERAGE_NUM_SAMPLES];
volatile int8_t zDataArray[XL_AVERAGE_NUM_SAMPLES];

bool buttonActive = false;

bool gestureLockedOut = false;

bool gestureDisplayReset = false;

bool continueUpdatingAdvertisementData = true;

bool deviceWokeUpStartCountdownToSleep = true;

volatile uint8_t gestureCounter = 0;
 
// Interrupt function declarations
void systick_isr(void);
void buttonRelease_isr(void);
void buttonPress_isr(void);
	
void interruptsInit(void);
void interruptsDeinit(void);

void setTimer_ir(void);
void getTimer_ir(void);
void resetTimer_ir(void);

void setButtonPress_ir(void);
void getButtonPress_ir(void);
void resetButtonPress_ir(void);

void setButtonRelease_ir(void);
void getButtonRelease_ir(void);
void resetButtonRelease_ir(void);

// Flag for detecting long (3s=) or short press
bool three_second_push = 0;

void LED_Blink(void);

// Manufacturer Specific Data ADV structure type
struct mnf_specific_data_ad_structure
{
    uint8_t ad_structure_size;
    uint8_t ad_structure_type;
    //uint8_t company_id[APP_AD_MSD_COMPANY_ID_LEN];
    uint8_t proprietary_data[APP_AD_MSD_DATA_LEN];
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

uint8_t app_connection_idx                      __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
timer_hnd app_adv_data_update_timer_used        __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
timer_hnd app_param_update_request_timer_used   __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
timer_hnd app_gesture_direction_timer_used      __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
timer_hnd app_gesture_display_reset_timer_used  __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
timer_hnd app_motor_on_timer_used  							__SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

// Retained variables
struct mnf_specific_data_ad_structure mnf_data  __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
// Index of manufacturer data in advertising data or scan response data (when MSB is 1)
uint8_t mnf_data_index                          __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t stored_adv_data_len                     __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t stored_scan_rsp_data_len                __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t stored_adv_data[ADV_DATA_LEN]           __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t stored_scan_rsp_data[SCAN_RSP_DATA_LEN] __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/



/**
 ****************************************************************************************
 * @brief Gesture display reset timer callback function.
 ****************************************************************************************
*/
static void gesture_display_reset_timer_cb()
{
	//arch_printf("RESETTING THE GESTURE COUNTER AND DISPLAY\n\r");	
	for(int i=0; i<APP_AD_MSD_DATA_NUM_BYTES;i++)
	{
		mnf_data.proprietary_data[i] = 0;
	}
	gestureCounter = 0;	
	gestureDisplayReset = false;
	arch_printf("gesture display reset called\n\r");
	user_app_going_to_sleep();
}

/**
 ****************************************************************************************
 * @brief Lockout the gesture recognition timer callback function.
 ****************************************************************************************
*/
static void gesture_lockout_timer_cb()
{
	gestureLockedOut = false;
}

/**
 ****************************************************************************************
 * @brief Initialize Manufacturer Specific Data
 ****************************************************************************************
 */
static void mnf_data_init()
{
    mnf_data.ad_structure_size = sizeof(struct mnf_specific_data_ad_structure ) - sizeof(uint8_t); // minus the size of the ad_structure_size field
    mnf_data.ad_structure_type = GAP_AD_TYPE_MANU_SPECIFIC_DATA;
    //mnf_data.company_id[0] = (APP_AD_MSD_COMPANY_ID >> 16 )& 0xFF; // MSB
    //mnf_data.company_id[1] = (APP_AD_MSD_COMPANY_ID >> 8 )& 0xFF; // MSB
    //mnf_data.company_id[2] = APP_AD_MSD_COMPANY_ID & 0xFF; // LSB
    mnf_data.proprietary_data[0] = 0;
    mnf_data.proprietary_data[1] = 0;
    mnf_data.proprietary_data[2] = 0;
    //mnf_data.proprietary_data[3] = 0;
		gestureCounter = 0;
		xlDataArrayCounter = 0;
				
}

/**
 ****************************************************************************************
 * @brief Update Manufacturer Specific Data
 ****************************************************************************************
 */
static void mnf_data_update()
{
    //uint16_t data;

//    data = mnf_data.proprietary_data[0] | (mnf_data.proprietary_data[1] << 8);
//    data += 1;
//    mnf_data.proprietary_data[0] = data & 0xFF;
//    mnf_data.proprietary_data[1] = (data >> 8) & 0xFF;

//    if (data == 0xFFFF) {
//         mnf_data.proprietary_data[0] = 0;
//         mnf_data.proprietary_data[1] = 0;
//    }
		
			//i2c_XL_Read_ALL();
	
//			mnf_data.proprietary_data[0] = (i2c_XL_Read_X()>>8); //0x86;
//			mnf_data.proprietary_data[1] = (i2c_XL_Read_Y()>>8); //0x24;
//			mnf_data.proprietary_data[2] = (i2c_XL_Read_Z()>>8); //0x28;
	
				int xData = 0;
				int	yData = 0;
				int zData = 0;

				for(int imCounting=0; imCounting<XL_AVERAGE_NUM_SAMPLES;imCounting++)
				{
						xDataArray[imCounting] *= 0.9;
						yDataArray[imCounting] *= 0.9;
						zDataArray[imCounting] *= 0.9;
				}

				xDataArray[xlDataArrayCounter] = (i2c_XL_Read_X()>>8);
				yDataArray[xlDataArrayCounter] = (i2c_XL_Read_Y()>>8);
				zDataArray[xlDataArrayCounter] = (i2c_XL_Read_Z()>>8);

				
				for(int i=0; i<XL_AVERAGE_NUM_SAMPLES;i++)
				{
					xData += xDataArray[i];
					yData += yDataArray[i];
					zData += zDataArray[i];
				}
				
				xData = (xData/XL_AVERAGE_NUM_SAMPLES);//(xData>>XL_AVERAGE_NUM_MULTIPLIER);
				yData = (yData/XL_AVERAGE_NUM_SAMPLES);//(yData>>XL_AVERAGE_NUM_MULTIPLIER);
				zData = (zData/XL_AVERAGE_NUM_SAMPLES);//(zData>>XL_AVERAGE_NUM_MULTIPLIER);
						
				//arch_printf("x:%d, y:%d, z:%d\n\r",xData,yData,zData);
				//arch_printf("x:%d, y:%d\n\r",xData,yData);
				
				if(xlDataArrayCounter<XL_AVERAGE_NUM_SAMPLES)
				{
					xlDataArrayCounter++;
				}
				else
				{
					xlDataArrayCounter = 0;
				}
	
	
//			int8_t xData = (i2c_XL_Read_X()>>8);
//			int8_t yData = (i2c_XL_Read_Y()>>8);
//			int8_t zData = (i2c_XL_Read_Z()>>8);
//			
//			int8_t zxAngle = ((atan2(zData,xData)*180)/3.1415)-90;
//			int8_t zyAngle = ((atan2(zData,yData)*180)/3.1415)-90;
//			int8_t xyAngle = ((atan2(xData,yData)*180)/3.1415)-90;
			
			int8_t deltaData[3];
			//i2c_XL_Vector_Delta(deltaData);
			//volatile int8_t threshholdValue = 70;
			volatile int8_t threshholdValue = 15;
			deltaData[0] = xData;
			deltaData[1] = yData;	
			volatile int8_t minMaxValue = 0;
			volatile int8_t largestDelta = 0;
			for(int i=0;i<2;i++)
			{		
				if(abs(deltaData[i])>minMaxValue)
				{
					minMaxValue = abs(deltaData[i]);
					largestDelta = i;
				}
			}
			deltaData[2] = largestDelta;
			//if((abs(deltaData[0])>threshholdValue)||(abs(deltaData[1])>threshholdValue)||(abs(deltaData[2])>threshholdValue))
			if((abs(deltaData[0])>threshholdValue)||(abs(deltaData[1])>threshholdValue))
			{
				if(!gestureLockedOut)
				{
					volatile uint8_t newGestureData = 0x00;
					switch(deltaData[2])
					{
						case 0:
							arch_printf("\n\r*****\n\r**X** x:%d, y:%d, z:%d, gestureSlot:%d\n\r*****\n\r\n\r",deltaData[0],deltaData[1],deltaData[2],gestureCounter);
							newGestureData = ((deltaData[0]>0)?0x4C:0x52);
							break;
						case 1:
							arch_printf("\n\r*****\n\r**Y** x:%d, y:%d, z:%d, gestureSlot:%d\n\r*****\n\r\n\r",deltaData[0],deltaData[1],deltaData[2],gestureCounter);
							newGestureData = ((deltaData[1]>0)?0x44:0x55);
							break;
//						case 2:
//							arch_printf("\n\r*****\n\r**Z** x:%d, y:%d, z:%d, gestureSlot:%d\n\r*****\n\r\n\r",deltaData[0],deltaData[1],deltaData[2],gestureCounter);
//							newGestureData = ((deltaData[2]>0)?0x55:0x44);
//							break;
						default:
							break;
					}
					mnf_data.proprietary_data[gestureCounter] = newGestureData;
					//arch_printf("mnfData[%d] = [%d]\n\r",gestureCounter,mnf_data.proprietary_data[gestureCounter]);
					if(gestureCounter<(APP_AD_MSD_DATA_NUM_BYTES-1))
					{
						gestureCounter++;
					}
					else
					{
						gestureCounter = 0;
					}
					gestureLockedOut = true;
					app_gesture_direction_timer_used = app_easy_timer(APP_GESTURE_LOCK_TO, gesture_lockout_timer_cb);
					if(gestureDisplayReset)
					{
							app_easy_timer_cancel(app_gesture_display_reset_timer_used);
					}
					app_gesture_display_reset_timer_used = app_easy_timer(APP_GESTURE_RESET_DISPLAY_TO, gesture_display_reset_timer_cb);
					gestureDisplayReset = true;
				}
			}
			if(deviceWokeUpStartCountdownToSleep)
			{		
					if(gestureDisplayReset)
					{
							app_easy_timer_cancel(app_gesture_display_reset_timer_used);
					}
					app_gesture_display_reset_timer_used = app_easy_timer(APP_GESTURE_RESET_DISPLAY_TO, gesture_display_reset_timer_cb);
					gestureDisplayReset = true;
					deviceWokeUpStartCountdownToSleep = false;
			}

			//arch_printf("x:%d, y:%d, z:%d\n\r",deltaData[0],deltaData[1],deltaData[2]);
			//arch_printf("xData:%d, yData:%d, zData:%d, upperBound:128, lowerBound:-128\n\r",xData,yData,zData);
			//arch_printf("xzAngle:%d, yzAngle:%d, xyAngle:%d, upperBound:120, lowerBound:-120\n\r",zxAngle,zyAngle,xyAngle);	
	
//			uint8_t uartDataLength = 7;
//			uint8_t uartData[uartDataLength];
//			
//			uartData[0] = mnf_data.proprietary_data[0];
//			uartData[1] = 0x2C;
//			uartData[2] = mnf_data.proprietary_data[1];
//			uartData[3] = 0x2C;
//			uartData[4] = mnf_data.proprietary_data[2];
//			uartData[5] = 0x0A;
//			uartData[6] = 0x0D;
//		
//			uart_write_buffer(UART2, uartData, sizeof(&uartData));
	
			//mnf_data.proprietary_data[3] = 0;
//			if(buttonActive)
//			{
//				mnf_data.proprietary_data[3] = 0x22;
//			}
//			else
//			{
//				mnf_data.proprietary_data[3] = 0x24;
//			}
}

/**
 ****************************************************************************************
 * @brief Add an AD structure in the Advertising or Scan Response Data of the
 *        GAPM_START_ADVERTISE_CMD parameter struct.
 * @param[in] cmd               GAPM_START_ADVERTISE_CMD parameter struct
 * @param[in] ad_struct_data    AD structure buffer
 * @param[in] ad_struct_len     AD structure length
 * @param[in] adv_connectable   Connectable advertising event or not. It controls whether
 *                              the advertising data use the full 31 bytes length or only
 *                              28 bytes (Document CCSv6 - Part 1.3 Flags).
 ****************************************************************************************
 */
static void app_add_ad_struct(struct gapm_start_advertise_cmd *cmd, void *ad_struct_data, uint8_t ad_struct_len, uint8_t adv_connectable)
{
    uint8_t adv_data_max_size = (adv_connectable) ? (ADV_DATA_LEN - 3) : (ADV_DATA_LEN);
    
    if ((adv_data_max_size - cmd->info.host.adv_data_len) >= ad_struct_len)
    {
        // Append manufacturer data to advertising data
        memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len], ad_struct_data, ad_struct_len);

        // Update Advertising Data Length
        cmd->info.host.adv_data_len += ad_struct_len;
        
        // Store index of manufacturer data which are included in the advertising data
        mnf_data_index = cmd->info.host.adv_data_len - sizeof(struct mnf_specific_data_ad_structure);
    }
    else if ((SCAN_RSP_DATA_LEN - cmd->info.host.scan_rsp_data_len) >= ad_struct_len)
    {
        // Append manufacturer data to scan response data
        memcpy(&cmd->info.host.scan_rsp_data[cmd->info.host.scan_rsp_data_len], ad_struct_data, ad_struct_len);

        // Update Scan Response Data Length
        cmd->info.host.scan_rsp_data_len += ad_struct_len;
        
        // Store index of manufacturer data which are included in the scan response data
        mnf_data_index = cmd->info.host.scan_rsp_data_len - sizeof(struct mnf_specific_data_ad_structure);
        // Mark that manufacturer data is in scan response and not advertising data
        mnf_data_index |= 0x80;
    }
    else
    {
        // Manufacturer Specific Data do not fit in either Advertising Data or Scan Response Data
        ASSERT_WARNING(0);
    }
    // Store advertising data length
    stored_adv_data_len = cmd->info.host.adv_data_len;
    // Store advertising data
    memcpy(stored_adv_data, cmd->info.host.adv_data, stored_adv_data_len);
    // Store scan response data length
    stored_scan_rsp_data_len = cmd->info.host.scan_rsp_data_len;
    // Store scan_response data
    memcpy(stored_scan_rsp_data, cmd->info.host.scan_rsp_data, stored_scan_rsp_data_len);
}

/**
 ****************************************************************************************
 * @brief Advertisement data update timer callback function.
 ****************************************************************************************
*/
static void adv_data_update_timer_cb()
{
    // If mnd_data_index has MSB set, manufacturer data is stored in scan response
    uint8_t *mnf_data_storage = (mnf_data_index & 0x80) ? stored_scan_rsp_data : stored_adv_data;

    // Update manufacturer data
    mnf_data_update();

    // Update the selected fields of the advertising data (manufacturer data)
    memcpy(mnf_data_storage + (mnf_data_index & 0x7F), &mnf_data, sizeof(struct mnf_specific_data_ad_structure));

    // Update advertising data on the fly
    app_easy_gap_update_adv_data(stored_adv_data, stored_adv_data_len, stored_scan_rsp_data, stored_scan_rsp_data_len);
    
    // Restart timer for the next advertising update
		if(continueUpdatingAdvertisementData)
		{
			app_adv_data_update_timer_used = app_easy_timer(APP_ADV_DATA_UPDATE_TO, adv_data_update_timer_cb);
		}
}

/**
 ****************************************************************************************
 * @brief Parameter update request timer callback function.
 ****************************************************************************************
*/
static void param_update_request_timer_cb()
{
    app_easy_gap_param_update_start(app_connection_idx);
    app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
}

void user_app_init(void)
{
    app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;  
		continueUpdatingAdvertisementData = true;
	
    // Initialize Manufacturer Specific Data
		user_wakeup_example_init();
    mnf_data_init();
		//interruptsInit();
    
    // Initialize Advertising and Scan Response Data
    memcpy(stored_adv_data, USER_ADVERTISE_DATA, USER_ADVERTISE_DATA_LEN);
    stored_adv_data_len = USER_ADVERTISE_DATA_LEN;
    memcpy(stored_scan_rsp_data, USER_ADVERTISE_SCAN_RESPONSE_DATA, USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN);
    stored_scan_rsp_data_len = USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN;
    
    default_app_on_init();
	
		i2c_XL_initialize();
		//user_app_waking_from_sleep();
}

void user_app_adv_start(void)
{
    // Schedule the next advertising data update
    app_adv_data_update_timer_used = app_easy_timer(APP_ADV_DATA_UPDATE_TO, adv_data_update_timer_cb);
    
    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_undirected_advertise_get_active();
    
    // Add manufacturer data to initial advertising or scan response data, if there is enough space
    app_add_ad_struct(cmd, &mnf_data, sizeof(struct mnf_specific_data_ad_structure), 1);

    app_easy_gap_undirected_advertise_start();
}

void user_app_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    if (app_env[connection_idx].conidx != GAP_INVALID_CONIDX)
    {
        app_connection_idx = connection_idx;

        // Stop the advertising data update timer
        app_easy_timer_cancel(app_adv_data_update_timer_used);

        // Check if the parameters of the established connection are the preferred ones.
        // If not then schedule a connection parameter update request.
        if ((param->con_interval < user_connection_param_conf.intv_min) ||
            (param->con_interval > user_connection_param_conf.intv_max) ||
            (param->con_latency != user_connection_param_conf.latency) ||
            (param->sup_to != user_connection_param_conf.time_out))
        {
            // Connection params are not these that we expect
            app_param_update_request_timer_used = app_easy_timer(APP_PARAM_UPDATE_REQUEST_TO, param_update_request_timer_cb);
        }
    }
    else
    {
        // No connection has been established, restart advertising
        user_app_adv_start();
    }

    default_app_on_connection(connection_idx, param);
}

void user_app_adv_undirect_complete(uint8_t status)
{
    // If advertising was canceled then update advertising data and start advertising again
    if (status == GAP_ERR_CANCELED)
    {
        user_app_adv_start();
    }
}

void user_app_disconnect(struct gapc_disconnect_ind const *param)
{
    // Cancel the parameter update request timer
    if (app_param_update_request_timer_used != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(app_param_update_request_timer_used);
        app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
    }
    // Update manufacturer data for the next advertsing event
    mnf_data_update();
    // Restart Advertising
    user_app_adv_start();
}

void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    switch(msgid)
    {
        case GAPC_PARAM_UPDATED_IND:
        {
            // Cast the "param" pointer to the appropriate message structure
            struct gapc_param_updated_ind const *msg_param = (struct gapc_param_updated_ind const *)(param);

            // Check if updated Conn Params filled to preferred ones
            if ((msg_param->con_interval >= user_connection_param_conf.intv_min) &&
                (msg_param->con_interval <= user_connection_param_conf.intv_max) &&
                (msg_param->con_latency == user_connection_param_conf.latency) &&
                (msg_param->sup_to == user_connection_param_conf.time_out))
            {
            }
        } break;

        default:
            break;
    }
}
/**
 ****************************************************************************************
 * @brief Gesture display reset timer callback function.
 ****************************************************************************************
*/
static void motor_on_timer_cb()
{
	arch_printf("Motor stopping\n\r");	
	GPIO_SetInactive(MOTOR_PORT, MOTOR_PIN);
	continueUpdatingAdvertisementData = true;
	app_adv_data_update_timer_used = app_easy_timer(APP_ADV_DATA_UPDATE_TO, adv_data_update_timer_cb);
	deviceWokeUpStartCountdownToSleep = true;
}

void user_app_waking_from_sleep(void)
{
	i2c_XL_initialize();
	arch_printf("Motor starting\n\r");	
	app_motor_on_timer_used = app_easy_timer(APP_MOTOR_ON_TO, motor_on_timer_cb);
	GPIO_SetActive(MOTOR_PORT, MOTOR_PIN);
}
void user_app_going_to_sleep(void)
{
	continueUpdatingAdvertisementData = false;
	i2c_XL_Sleep_Mode();
	arch_printf("\n\rSystem going to sleep",NULL);

	app_easy_gap_advertise_stop();

	arch_set_sleep_mode(ARCH_EXT_SLEEP_ON);				
	arch_ble_ext_wakeup_on();
	user_reset_event_counter(); // When callback is triggerd the event counter is not set 0 for the 531, that is why this function is called. 		
}                                                               

///**
// ****************************************************************************************
// * @brief Toggles LED (Light Emitting Diode) GPIO (General Purpose In/Out) using systick().
// * @brief Toggle time = BLINK_INTERVAL_T (time in us).
// * @return void
// ****************************************************************************************
// */
//void LED_Blink(void)
//{
//		systick_stop();
//		systick_register_callback(LED_Blink);
//		if (GPIO_GetPinStatus(LED_PORT, LED_PIN))
//		{
//				GPIO_SetInactive(LED_PORT, LED_PIN);
//				//printf_string(UART,"\n\r=> LED OFF <=");
//		}
//		else
//		{
//				GPIO_SetActive(LED_PORT, LED_PIN);
//				//printf_string(UART,"\n\r=> LED ON  <=");
//		}
//		systick_start(BLINK_INTERVAL, true);
//}

///**
// ****************************************************************************************
// * @brief	systick ISR (Interrupt Service Routine) handler.
// * @return	void
// ****************************************************************************************
// */
//void systick_isr(void) 
//{
//		systick_stop();
//		three_second_push = 1;
//		//printf_string(UART,"\n\n\rLong Press");
//		LED_Blink();
//}

///**
// ****************************************************************************************
// * @brief Button release ISR (Interrupt Service Routine) handler.
// * @return void
// ****************************************************************************************
// */
//void buttonRelease_isr(void)
//{
//		// Prevents interrupt from triggering at startup
//		if (GPIO_GetIRQInputLevel(GPIO1_IRQn) == GPIO_IRQ_INPUT_LEVEL_LOW)
//		{
//				GPIO_SetIRQInputLevel(GPIO1_IRQn, GPIO_IRQ_INPUT_LEVEL_HIGH);
//				return;
//		}

//		if (!three_second_push)
//		{
//			systick_stop();
//			//printf_string(UART,"\n\n\rShort press");
//				if (GPIO_GetPinStatus(LED_PORT, LED_PIN))
//				{
//						GPIO_SetInactive(LED_PORT, LED_PIN);
//						buttonActive = false;
//						//printf_string(UART,"\n\r=> LED OFF <=");
//				}
//				else
//				{
//						GPIO_SetActive(LED_PORT, LED_PIN);
//						buttonActive = true;
//						//printf_string(UART,"\n\r=> LED ON  <=");
//				}
//		}
//}

///**
// ****************************************************************************************
// * @brief Button press ISR (Interrupt Service Routine) handler.
// * @brief Generates a timer. If LONG_PRESS_TIME time is passed an exception is generated.
// * @return void
// ****************************************************************************************
// */
//void buttonPress_isr(void)
//{
//		three_second_push = 0; 																													
//		systick_stop(); 																																				
//		systick_register_callback(systick_isr);																			
//		systick_start(LONG_PRESS_TIME, true);		
//}
//	
///**
// ****************************************************************************************
// * @brief Interrupts initialization.
// * @return void
// ****************************************************************************************
// */
//void interruptsInit(void)
//{
//		setTimer_ir();
//		setButtonPress_ir();
//		setButtonRelease_ir();
//}

///**
// ****************************************************************************************
// * @brief Interrupts deinitialization. For future use.
// * @return void
// ****************************************************************************************
// */
//void interruptsDeinit(void)
//{
//}

///**
// ****************************************************************************************
// * @brief Set timer IR (interrupt).
// * @return void
// ****************************************************************************************
// */
//void setTimer_ir(void) 
//{
//		systick_register_callback(systick_isr);
//}

///**
// ****************************************************************************************
// * @brief Get timer IR (interrupt). For future use.
// * @return void
// ****************************************************************************************
// */
//void getTimer_ir(void) 
//{
//}

///**
// ****************************************************************************************
// * @brief Reset timer IR (interrupt). For future use.
// * @return void
// ****************************************************************************************
// */
//void resetTimer_ir(void)
//{
//}

///**
// ****************************************************************************************
// * @brief Set button (GPIO) press IR (interrupt).
// * @return void
// ****************************************************************************************
// */
//void setButtonPress_ir(void)
//{
//		GPIO_EnableIRQ(GPIO_BUTTON_PORT, GPIO_BUTTON_PIN, GPIO0_IRQn, INPUT_LEVEL, EDGE_LEVEL, DEBOUNCE_MS);
//		GPIO_SetIRQInputLevel(GPIO0_IRQn, GPIO_IRQ_INPUT_LEVEL_LOW);
//		GPIO_RegisterCallback(GPIO0_IRQn, buttonPress_isr);
//}

///**
// ****************************************************************************************
// * @brief Get button (GPIO) press IR (interrupt). For future use.
// * @return void
// ****************************************************************************************
// */
//void getButtonPress_ir(void)
//{
//}

///**
// ****************************************************************************************
// * @brief Reset button (GPIO) press IR (interrupt). For future use.
// * @return void
// ****************************************************************************************
// */
//void resetButtonPress_ir(void)
//{
//}

///**
// ****************************************************************************************
// * @brief Set button (GPIO) release IR (interrupt).
// * @return void
// ****************************************************************************************
// */
//void setButtonRelease_ir(void)
//{
//		GPIO_EnableIRQ(GPIO_BUTTON_PORT, GPIO_BUTTON_PIN, GPIO1_IRQn, INPUT_LEVEL, EDGE_LEVEL, DEBOUNCE_MS);
//		GPIO_SetIRQInputLevel(GPIO1_IRQn, GPIO_IRQ_INPUT_LEVEL_LOW);
//		GPIO_RegisterCallback(GPIO1_IRQn, buttonRelease_isr);
//}

///**
// ****************************************************************************************
// * @brief Get button (GPIO) release IR (interrupt). For future use.
// * @return void
// ****************************************************************************************
// */
//void GetButtonRelease_ir(void)
//{
//}

///**
// ****************************************************************************************
// * @brief Reset button (GPIO) release IR (interrupt). For future use.
// * @return void
// ****************************************************************************************
// */
//void resetButtonRelease_ir(void)
//{
//}

/// @} APP
