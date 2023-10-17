/*******************************************************************************
// Error Service Configuration
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Error_Mgr.h"
#include "Error_Mgr_Config.h"
#include "Error_Mgr_ConfigTypes.h" // Defines configuration structure
// Platform Includes
#include "MessageRouter.h"
// Other Includes


/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The table defining all the list of critical errors defined for the system.
const Error_Mgr_Data_t errorData[] =
{
    // TODO - Generic placeholders
   { ERROR_MGR_ERROR_CRITICAL },
   { ERROR_MGR_ERROR_OVER_CURRENT },
   { ERROR_MGR_ERROR_OVER_VOLTAGE },
   { ERROR_MGR_ERROR_OVER_TEMP },
   { ERROR_MGR_ERROR_GATE_DRIVER },
};


// Common configuration structure passed to the module initialization function
extern const Error_Mgr_Config_t errorConfig =
{
     // The number of items in the errorConfigData - calculated by compiler
    .numConfigItems = sizeof(errorData)/sizeof(Error_Mgr_Data_t),
    // Defines the list of errors that are considered "Critical"
    // this is used to identify errors that trigger an immediate shutdown
    .dataPtr = errorData
};


// This table provides a list of commands for this module.
const MessageRouter_CommandTableItem_t errorMessageTable[] =
{
   // {Command ID, Message Handler Function Pointer}
   { 1, Error_Mgr_MessageRouter_GetErrorState },
   { 2, Error_Mgr_MessageRouter_SetErrorState },
   { 3, Error_Mgr_MessageRouter_DoErrorsExist },
   { 4, Error_Mgr_MessageRouter_ClearAllErrors },
   { 5, Error_Mgr_MessageRouter_GetAllErrors },
   { 6, Error_Mgr_MessageRouter_GetErrorDetails }
};

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


/*******************************************************************************
// Public Function Implementations
*******************************************************************************/


