/*******************************************************************************
// Timebase Service Interface
// Provides system-wide timebase used for software timers and scheduling.
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once


/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Timebase_Config.h"
// Platform Includes
// Other Includes
#include <stdbool.h> // Defines C99 boolean type


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

// The maximum allowable Tick value (simply max unsigned 32-bit value)
#define TIMEBASE_MAX_TICK_VALUE (UINT32_MAX)


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

// Defines type used to represent internal system tick in API functions
typedef uint32_t Timebase_Tick_t;


/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/*******************************************************************************
// Description:
//    Intialize the timebase module that is define system time used for scheduling
//    and software timers.
// Parameters:
//    moduleId - The numeric module identifier to be used for the module.
//       Note that this value should be unique to to each module in the system.
//    configData - Defines the required configuration data for the module.
//       Note that the data is typically defined in the provided as part of the
//       board-specific configuration files.
// Returns: 
//    bool - The result of the initialization
// Return Value List: 
//    true - The module was initialized successfully
//    false - The module was unable to complete successful initialization.
//    Calls to API functions in the module will have unxepcted results.
*******************************************************************************/
bool Timebase_Init(const uint32_t moduleID, const void *configData);


/*******************************************************************************
// Description:
//    This function retrieves the current value of the Timebase.  In most
//    implementations, this will just be the value of the SysTick timer.
// Parameters:
//    none
// Returns:
//    Timebase_Tick_t - The current 32-bit tick count for the system
*******************************************************************************/
Timebase_Tick_t Timebase_GetCurrentTickCount(void);

uint32_t Timebase_TicksToMilliseconds(Timebase_Tick_t const tickCount);
Timebase_Tick_t Timebase_CalculateElapsedTimeTicks(const Timebase_Tick_t startTickCount, const Timebase_Tick_t endTickCount);

/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif

