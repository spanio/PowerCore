/*******************************************************************************
// LED Mgr Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "LED_Mgr_Config.h"
// Platform Includes
#include "GPIO_Drv.h"
// Other Includes
#include <stdbool.h>
#include <stdint.h> // Defines C99 integer types


/*******************************************************************************
// Start C Binding Section for C++ Compilers
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/*******************************************************************************
// Public Constant Definitions
*******************************************************************************/


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

typedef struct
{
    // GPIO Output Channel
    GPIO_Drv_ChannelId_t gpioChannelId;
    // LED Polarity
    bool isActiveHigh;
} LED_Mgr_GPIO_Configuration_t;

typedef struct LED_Mgr_Data_s
{
    // LED Channel
    LED_Mgr_ChannelId_t ledChannelId;
    //GPIO configuration
    LED_Mgr_GPIO_Configuration_t configuration;
} LED_Mgr_ChannelConfig_t;


typedef struct LED_Mgr_ConfigSettings_s {
    uint32_t numConfigItems;
    const LED_Mgr_ChannelConfig_t *configSettings;
} LED_Mgr_ConfigSettings_t;


// End of C Binding Section
#ifdef __cplusplus
}
#endif

