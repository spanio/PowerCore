/*******************************************************************************
// Timebase Driver Configuration
*******************************************************************************/

// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
#include "SysTick_Drv.h"
// Other Includes
#include <stdint.h>  // Define C99 integer types

// Start C Binding Section for C++ Compilers
#ifdef __cplusplus
extern "C"
{
#endif


/*******************************************************************************
// Public Constant Definitions
*******************************************************************************/

#define TIMEBASE_NUM_TICKS_PER_MILLISECOND (SYSTICK_DRV_NUM_TICKS_PER_SECOND/UINT32_C(1000))


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

// Defines type used to represent internal timebase tick used in API functions
typedef uint32_t Timebase_Drv_Tick_t;


/*******************************************************************************
// Public Variable Definitions
*******************************************************************************/

// Defines the 32-bit System Tick value used for system timing.  This variable 
// is defined globally to prevent the overhead of a function call to get the 
// value.  This value is only modified by the SysTick Driver. Optionally, the
// SysTick_Drv_GetCurrentTickCount can be called to return this value.
extern volatile Timebase_Drv_Tick_t Timebase_Drv_tickCount;


// End of C Binding Section
#ifdef __cplusplus
}
#endif

