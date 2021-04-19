#ifndef _I2C_XL_H_
#define _I2C_XL_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"
/*
 * DEFINES
 ****************************************************************************************
 */ 
 #define XL_ADDRESS 0x18
 
 #define XL_WHO_AM_I 0x0F
 #define XL_CONTROL_REG_0 0x1E
 #define XL_CONTROL_REG_1 0x20
 #define XL_CONTROL_REG_2 0x21
 #define XL_CONTROL_REG_3 0x22
 #define XL_CONTROL_REG_4 0x23
 #define XL_CONTROL_REG_5 0x24
 #define XL_CONTROL_REG_6 0x25
 #define XL_OUT_X_L				0x28
 #define XL_OUT_X_H				0x29
 #define XL_OUT_Y_L				0x2A
 #define XL_OUT_Y_H				0x2B
 #define XL_OUT_Z_L				0x2C
 #define XL_OUT_Z_H				0x2D
 #define XL_INT1_CFG			0x30
 #define XL_INT1_THS			0x32
 #define XL_INT1_DUR			0x33
 
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

 /**
 ****************************************************************************************
 * @brief Initialize I2C Accelerometer
 ****************************************************************************************
 */
void i2c_XL_initialize(void);

 /**
 ****************************************************************************************
 * @brief Read X-axis I2C Accelerometer
 ****************************************************************************************
 */
uint16_t i2c_XL_Read_X(void);

 /**
 ****************************************************************************************
 * @brief Read Y-axis I2C Accelerometer
 ****************************************************************************************
 */
uint16_t i2c_XL_Read_Y(void);

 /**
 ****************************************************************************************
 * @brief Read Z-axis I2C Accelerometer
 ****************************************************************************************
 */
uint16_t i2c_XL_Read_Z(void);

 /**
 ****************************************************************************************
 * @brief Read all-axis I2C Accelerometer
 * @input 4 element array
 * @output cells 0-2 are data, cell 3 is largest vector
 ****************************************************************************************
 */
void i2c_XL_Read_ALL(void);


 /**
 ****************************************************************************************
 * @brief Get largest vector direction of XL
 * @input 3 cell array (x,y,z)
 ****************************************************************************************
 */
void i2c_XL_Vector_Delta(int8_t *vectorData);

 /**
 ****************************************************************************************
 * @brief Set XL to low power state with interrupt enabled
 ****************************************************************************************
 */
void i2c_XL_Sleep_Mode(void);
#endif // _I2C_XL_H_

///@}
///@}
