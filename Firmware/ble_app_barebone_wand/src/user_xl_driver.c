/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "compiler.h"
#include "ll.h"
#include "user_xl_driver.h"
#include "i2c.h"
#include "stdlib.h"


volatile int8_t previousData[3];

 /**
 ****************************************************************************************
 * @brief Initialize I2C Accelerometer
 ****************************************************************************************
 */
void i2c_XL_initialize(void)
	{
	uint8_t registerToSend[2];
	uint8_t xlData[2];
	i2c_abort_t abrt_code;
	//Disable controller to change slave address 
	i2c_set_controller_status(I2C_CONTROLLER_DISABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_DISABLE));
	i2c_set_target_address(XL_ADDRESS);
	i2c_set_controller_status(I2C_CONTROLLER_ENABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_ENABLE));
		
	registerToSend[0] = XL_CONTROL_REG_1;
	registerToSend[1] = 0x57;
	i2c_master_transmit_buffer_sync(registerToSend,sizeof(registerToSend),&abrt_code,I2C_F_ADD_STOP);
	
	//Set to off
	registerToSend[0] = XL_INT1_CFG;
	registerToSend[1] = 0x00;
	i2c_master_transmit_buffer_sync(registerToSend,sizeof(registerToSend),&abrt_code,I2C_F_ADD_STOP);
		
	i2c_master_transmit_buffer_sync(registerToSend,sizeof(registerToSend),&abrt_code,I2C_F_NONE);
	i2c_master_receive_buffer_sync(&xlData[0], sizeof(&xlData[0]), &abrt_code, I2C_F_ADD_STOP);
	previousData[0] = 0;
	previousData[1] = 0;
	previousData[2] = 0;
 }
	
 /**
 ****************************************************************************************
 * @brief Read X-axis I2C Accelerometer
 ****************************************************************************************
 */
uint16_t i2c_XL_Read_X(void)
	{
	uint8_t registerToSend[2];
	uint8_t xlData[2];

	i2c_abort_t abrt_code;
	volatile uint16_t returnValue = 0;
	//Disable controller to change slave address 
	i2c_set_controller_status(I2C_CONTROLLER_DISABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_DISABLE));
	i2c_set_target_address(XL_ADDRESS);
	i2c_set_controller_status(I2C_CONTROLLER_ENABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_ENABLE));
		
	registerToSend[0] = XL_OUT_X_L;
	registerToSend[1] = XL_OUT_X_H;
	
	i2c_master_transmit_buffer_sync(&registerToSend[0],sizeof(&registerToSend[0]),&abrt_code,I2C_F_NONE);
	i2c_master_receive_buffer_sync(&xlData[0], sizeof(&xlData[0]), &abrt_code, I2C_F_ADD_STOP);
	returnValue = xlData[0];
		
	i2c_master_transmit_buffer_sync(&registerToSend[1],sizeof(&registerToSend[1]),&abrt_code,I2C_F_NONE);
	i2c_master_receive_buffer_sync(&xlData[1], sizeof(&xlData[1]), &abrt_code, I2C_F_ADD_STOP);
	returnValue |= xlData[1] << 8;
		
	return returnValue;
 }
	
 /**
 ****************************************************************************************
 * @brief Read Y-axis I2C Accelerometer
 ****************************************************************************************
 */
uint16_t i2c_XL_Read_Y(void)
	{
	uint8_t registerToSend[2];
	uint8_t xlData[2];

	i2c_abort_t abrt_code;
	volatile uint16_t returnValue = 0;
	//Disable controller to change slave address 
	i2c_set_controller_status(I2C_CONTROLLER_DISABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_DISABLE));
	i2c_set_target_address(XL_ADDRESS);
	i2c_set_controller_status(I2C_CONTROLLER_ENABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_ENABLE));
		
	registerToSend[0] = XL_OUT_Y_L;
	registerToSend[1] = XL_OUT_Y_H;
		
	i2c_master_transmit_buffer_sync(&registerToSend[0],sizeof(&registerToSend[0]),&abrt_code,I2C_F_NONE);
	i2c_master_receive_buffer_sync(&xlData[0], sizeof(&xlData[0]), &abrt_code, I2C_F_ADD_STOP);
	returnValue = xlData[0];
		
	i2c_master_transmit_buffer_sync(&registerToSend[1],sizeof(&registerToSend[1]),&abrt_code,I2C_F_NONE);
	i2c_master_receive_buffer_sync(&xlData[1], sizeof(&xlData[1]), &abrt_code, I2C_F_ADD_STOP);
	returnValue |= xlData[1] << 8;
		
	return returnValue;
 }
	
 /**
 ****************************************************************************************
 * @brief Read Z-axis I2C Accelerometer
 ****************************************************************************************
 */
uint16_t i2c_XL_Read_Z(void)
	{
	uint8_t registerToSend[2];
	uint8_t xlData[2];

	i2c_abort_t abrt_code;
	volatile uint16_t returnValue = 0;
	//Disable controller to change slave address 
	i2c_set_controller_status(I2C_CONTROLLER_DISABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_DISABLE));
	i2c_set_target_address(XL_ADDRESS);
	i2c_set_controller_status(I2C_CONTROLLER_ENABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_ENABLE));
		
	registerToSend[0] = XL_OUT_Z_L;
	registerToSend[1] = XL_OUT_Z_H;
		
	i2c_master_transmit_buffer_sync(&registerToSend[0],sizeof(&registerToSend[0]),&abrt_code,I2C_F_NONE);
	i2c_master_receive_buffer_sync(&xlData[0], sizeof(&xlData[0]), &abrt_code, I2C_F_ADD_STOP);
	returnValue = xlData[0];
		
	i2c_master_transmit_buffer_sync(&registerToSend[1],sizeof(&registerToSend[1]),&abrt_code,I2C_F_NONE);
	i2c_master_receive_buffer_sync(&xlData[1], sizeof(&xlData[1]), &abrt_code, I2C_F_ADD_STOP);
	returnValue |= xlData[1] << 8;
		
	return returnValue;
 }
	
 /**
 ****************************************************************************************
 * @brief Read all-axis I2C Accelerometer
 ****************************************************************************************
 */
void i2c_XL_Read_ALL(void)
{
	volatile int16_t xlOutData[3] = {0,0,0};

	xlOutData[0] = i2c_XL_Read_X();
	xlOutData[1] = i2c_XL_Read_Y();
	xlOutData[2] = i2c_XL_Read_Z();
}

 /**
 ****************************************************************************************
 * @brief Get largest vector direction of XL
 * @input 4 cell array (x,y,z,largest delta)
 ****************************************************************************************
 */
void i2c_XL_Vector_Delta(int8_t *vectorData)
{	
	volatile int8_t xlOutData[3] = {0,0,0};
	volatile int8_t minMaxValue = 0;
	volatile int8_t largestDelta = 0;

	xlOutData[0] = (i2c_XL_Read_X()>>8);
	xlOutData[1] = (i2c_XL_Read_Y()>>8);
	xlOutData[2] = (i2c_XL_Read_Z()>>8);
	
	for(int i=0;i<3;i++)
	{		
		vectorData[i] = xlOutData[i]-previousData[i];	
		previousData[i] = xlOutData[i];
		if(abs(vectorData[i])>minMaxValue)
		{
			minMaxValue = abs(vectorData[i]);
			largestDelta = i;
		}
	}
	vectorData[3] = largestDelta;
}

 /**
 ****************************************************************************************
 * @brief Set XL to low power state with interrupt enabled
 ****************************************************************************************
 */
void i2c_XL_Sleep_Mode(void)
{
	uint8_t registerToSend[2];
	i2c_abort_t abrt_code;
	//Disable controller to change slave address 
	i2c_set_controller_status(I2C_CONTROLLER_DISABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_DISABLE));
	i2c_set_target_address(XL_ADDRESS);
	i2c_set_controller_status(I2C_CONTROLLER_ENABLE);
	while ((i2c_get_controller_status() != I2C_CONTROLLER_ENABLE));
		 
	//Set lowest sampling rate, low power mode, and only Y axis enabled
	registerToSend[0] = XL_CONTROL_REG_1;
	registerToSend[1] = 0x1A;
	i2c_master_transmit_buffer_sync(registerToSend,sizeof(registerToSend),&abrt_code,I2C_F_ADD_STOP);
	
	//Set Interrupt Active (1) onto Interrupt Pin 2 and setting interrupt to Active Low 
	registerToSend[0] = XL_CONTROL_REG_6;
	registerToSend[1] = 0x42;
	i2c_master_transmit_buffer_sync(registerToSend,sizeof(registerToSend),&abrt_code,I2C_F_ADD_STOP);
	
	//Set Interrupt to only fire on Y+ event
	registerToSend[0] = XL_INT1_CFG;
	registerToSend[1] = 0x88;
	i2c_master_transmit_buffer_sync(registerToSend,sizeof(registerToSend),&abrt_code,I2C_F_ADD_STOP);
	
	//Set Interrupt threshhold
	registerToSend[0] = XL_INT1_THS;
	registerToSend[1] = 0x20;
	i2c_master_transmit_buffer_sync(registerToSend,sizeof(registerToSend),&abrt_code,I2C_F_ADD_STOP);
	
	//Set duration for interrupt to be recognnized
	registerToSend[0] = XL_INT1_DUR;
	registerToSend[1] = 0x01;
	i2c_master_transmit_buffer_sync(registerToSend,sizeof(registerToSend),&abrt_code,I2C_F_ADD_STOP);
} 

 