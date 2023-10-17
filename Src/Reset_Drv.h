/*******************************************************************************
// Reset Driver
// Defines the common interface for restting the CPU and quering the reset 
// reason(s) as supported by the platform implementation. 
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
#include "Reset_Drv_Config.h"
#include "Reset_Drv_ConfigTypes.h"
// Other Includes
#include <stdbool.h> // Defines C99 boolean type
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

// Common configuration structure passed to the module initialization function
// Data is generally defined in the board-specific configuration file
typedef struct
{
    // The number of items in the resetConfigData - calculated by compiler
    uint32_t numConfigItems;
    // Configuration data
    const struct Reset_Drv_Config_Data_s *dataPtr;
} Reset_Drv_Config_t;

/*******************************************************************************
// External Variable Declarations
*******************************************************************************/

// Required configuration structure to be passed during Init
extern const Reset_Drv_Config_t resetConfig;

/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/*******************************************************************************
// Description:
//    Initialize the CPU reset driver.  The previous reset reason is stored
//    for later query by the Reset_Drv_GetResetReason function.
//    Since the available reasons vary for each CPU, each platform may have
//    a unique list of reasons for reset.
// Parameters:
//    moduleId - The numeric module identifier to be used for the module.
//       Note that this value should be unique to to each module in the system.
//    configPtr - Defines the required configuration data for the module.
//       Note that the data is typically defined in the provided as part of the
//       board-specific configuration files.
// Returns: 
//    bool - The result of the initialization
// Return Value List: 
//    true - The module was initialized successfully
//    false - The module was unable to complete successful initialization.
//    Calls to API functions in the module will have unexpected results.
*******************************************************************************/
bool Reset_Drv_Init(const uint32_t moduleId, const Reset_Drv_Config_t *configPtr);


/*******************************************************************************
// Description:
//    Requests a device reset as soon as possible.  The implementation may
//    may choose to do an immediate reset or a controlled shutdown.
// Parameters:
//    none
// Returns: 
//    none
*******************************************************************************/
void Reset_Drv_RequestReset(void);


/*******************************************************************************
// Description:
//    Queries the reason for the last reset.
// Parameters:
//    none
// Returns: 
//    uint32_t - A 32-bit value representing the platform-speccific reason for
//    last CPU reset. This value is different for each platform and may
//    represent a logical OR of multiple values.
*******************************************************************************/
uint32_t Reset_Drv_GetResetReason(void);


/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif


