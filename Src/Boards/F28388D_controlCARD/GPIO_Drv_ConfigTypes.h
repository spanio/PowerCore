/*******************************************************************************
// GPIO Driver Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "GPIO_Drv_Config.h"
// Platform Includes
#include "Port_Drv_Config.h"
// Other Includes
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


typedef struct GPIO_Drv_Data_s {
    // Logical channel identifier
    // Note that this is expected to match the ADC_Drv_Channel_t enumeration such that
    // the first item configuration corresponds to the the first enumerated value
    GPIO_Drv_ChannelId_t channelId;
    Port_Drv_PortId_t portId;
    Port_Drv_Port_Pin_t pinIndex;
} GPIO_Drv_Data_t;

typedef struct GPIO_Drv_ConfigSettings_s {
    uint32_t numConfigItems;
    const GPIO_Drv_Data_t *configSettings;
} GPIO_Drv_ConfigSettings_t;


// End of C Binding Section
#ifdef __cplusplus
}
#endif

