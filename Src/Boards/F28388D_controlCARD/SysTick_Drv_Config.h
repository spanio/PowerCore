/*******************************************************************************
// System Tick Driver Configuration
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
#include "Interrupt_Drv.h" // ISR function
#include "Sys_Config.h" // Clock defines
// Other Includes
#include <stdint.h>  // Defines C99 integer types


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

// Configure system to use 1000 ticks per second (each tick = 1ms)
#define SYSTICK_DRV_NUM_TICKS_PER_SECOND (UINT32_C(1000))

// Defines the SysTick IRQ Handler for this platform (CPU Timer 2)
//#define SYSTICK_DRV_IRQ_HANDLER cpuTimer2ISR


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

typedef enum {
   SYSTICK_DRV_CHANNEL_ID_TIMER2,
   SYSTICK_DRV_CHANNEL_ID_COUNT
} SysTick_Drv_ChannelId_t;


// Defines type used to represent internal system tick in API functions
typedef uint32_t SysTick_Drv_Tick_t;


/*******************************************************************************
// Public Variable Definitions
*******************************************************************************/

// Defines the 32-bit System Tick value used for system timing.  This variable 
// is defined globally to prevent the overhead of a function call to get the 
// value.  This value is only modified by the SysTick Driver. Optionally, the
// SysTick_Drv_GetCurrentTickCount can be called to return this value.
extern volatile SysTick_Drv_Tick_t SysTick_Drv_sysTickCount;


/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif

