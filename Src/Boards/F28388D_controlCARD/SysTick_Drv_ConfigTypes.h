/*******************************************************************************
// SysTick Driver Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "SysTick_Drv_Config.h"
// Platform Includes
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

typedef struct SysTick_Drv_Channel_Config_s {
    SysTick_Drv_ChannelId_t channelId;
    uint32_t timerBase;
    SysCtl_PeripheralPCLOCKCR peripheral;
    uint32_t interruptNumber;
    // TODO: Should this be ISR or configure for polled interface
    void *callback;
} SysTick_Drv_Channel_Config_t;


// End of C Binding Section
#ifdef __cplusplus
}
#endif

