/*******************************************************************************
// Reset Driver - TI F2838x Implementation
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Reset_Drv.h" // Module header
#include "Reset_Drv_Config.h" // Module header
#include "Reset_Drv_ConfigTypes.h" // Module header
// Platform Includes
// Other Includes
#include <stdint.h> // Defines C99 integer types
#include "sysctl.h" // TI System Control functions for resetting


/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

// This structure defines the internal variables used by the module
typedef struct
{
    uint32_t moduleId;

    // Initialization state for the module
    bool isInitialized;

   // Hold the value of the last reset reason(s)
   uint32_t deviceResetReason;

   // Configuration Table passed at Initialization
   const Reset_Drv_Config_t *resetConfig;
} Reset_Status_t;


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The variable used for holding all internal data for this module.
static Reset_Status_t status;


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

// Initializes the module
bool Reset_Drv_Init(const uint32_t moduleId, const Reset_Drv_Config_t *configPtr)
{
   // Get the last reason for reset from the C28x system control driver
   status.deviceResetReason = SysCtl_getResetCause();

   // Default module to uninitialized
   status.isInitialized = false;
   // Store the module Id for error reporting
   status.moduleId = moduleId;

   // First, validate the given parameter is valid
   if ((NULL != configPtr) && (NULL != configPtr->dataPtr))
   {
       // Store the given configuration table
       status.resetConfig = (Reset_Drv_Config_t *)configPtr;

       //-----------------------------------------------
       // Local Variable Initialization
       //-----------------------------------------------

       // Start with no errors to reset
       uint32_t resetMask = 0UL;

       // Loop through configuration and build the list of reset reasons to be automatically cleared at startup
       // From TI docuemntation, all reason should be cleared after reset reasons have been stored
       for (int i = 0; i < status.resetConfig->numConfigItems; i++)
       {
           // clearResetCause documentations says this should be logical OR but it appears that it should be a bitwise OR
           resetMask = resetMask | status.resetConfig->dataPtr[i].resetCause;
       }

       // Clear the reset reasons if mask is set
       if (0UL != resetMask)
       {
           // Just pass the mask directly to the TI driverLib
           SysCtl_clearResetCause(resetMask);
       }

       // Once cleared, consider initialization completed
       status.isInitialized = true;
   }

   // Init complete
   return(status.isInitialized);
}

// Call the reset function from the TI System Control driver
void  Reset_Drv_RequestReset(void)
{
    // The system control driver performs a reset by writing an incorrect check value to the watchdog control register
    // Note that this will enable the watchdog and enter an infinite loop until the reset occurs
    SysCtl_resetDevice();
}

// Return the stored reason for the last reset
uint32_t Reset_Drv_GetResetReason(void)
{
   // Simply return the stored reset reason
   return(status.deviceResetReason);
}

