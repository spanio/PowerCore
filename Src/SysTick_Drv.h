/*******************************************************************************
// System Tick Driver
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "SysTick_Drv_Config.h" // Module configuration
// Platform Includes
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


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

// SysTick_Drv_Tick_t type defined in configuration file

// Defines the type passed during initialization that specified the SysTick configuration
typedef struct
{
    // The number of items defined by channelConfigArray
    uint32_t numConfigItems;
    // The configuration for each of the CPU timers to be configured
    const struct SysTick_Drv_Channel_Config_s *dataPtr;
} SysTick_Drv_Config_t;


/*******************************************************************************
// Public Variable Definition
*******************************************************************************/


/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/*******************************************************************************
// Description:
//    Specifies the configuration tables used for the module and also specifies
//    the unique module Identifier used for routing and error reporting.
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
bool SysTick_Drv_Init(const uint32_t moduleID, const SysTick_Drv_Config_t *configData);

/*******************************************************************************
// Description:
//    Specifies the configuration tables used for the module and also specifies
//    the unique module Identifier used for routing and error reporting.
// Parameters:
//    enbleState - Defines the desired state for the SysTick timer.
//    (true: Enable, false: Disable)
// Returns: 
//    none
*******************************************************************************/
void SysTick_Drv_SetEnableState(bool enableState);

/*******************************************************************************
// Description:
//    Fetches the current value of the SysTick counter.
// Parameters: 
//    none 
// Returns: 
//    SysTick_Drv_Tick_t - The current value 32-bit SysTick variable
*******************************************************************************/
SysTick_Drv_Tick_t SysTick_Drv_GetCurrentTickCount(void);

// IRQ for CPU Timer 2 - Used for System Tick
//TODO - INTERRUPT_FUNC void SysTick_Handler(void);
__interrupt void SysTick_Handler(void);


/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif

