/*******************************************************************************
// System Driver Configuration Data
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "Sys.h"
#include "Sys_Config.h" // CPU enumeration
#include "Sys_ConfigTypes.h" // Defines configuration structure
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

const Sys_Data_t sysConfigData[SYS_CPU_ID_COUNT] = {

    {
     .cpuId = SYS_CPU_ID_1,
     .productConfig =
     {
         // Define version of current system
         .version = {
            .major = 0,
            .minor = 0,
            .build = 1,
         },
         // Define machine-readable Product Id for the system
         // Currently we use it to define if build is for F28379 or F28388 processor
         .productId = 28388U,
          // Define configuration used for system
          // 1 = Release Configuration, 0 = Debug Configuration (Default)
      #ifdef RELEASE_TARGET
          .releaseConfiguration = SYS_RELEASE_CONFIGURATION_RELEASE,
      #else
          .releaseConfiguration = SYS_RELEASE_CONFIGURATION_DEBUG,
      #endif
         // Define human-readable Product Name for the system
         .productName = {
            .stringValue = "PowerCore"
         }
        }
    }
};

// Common configuration structure passed to the module initialization function
extern const Sys_Config_t sysConfig =
{
     // The number of items in the sysConfigData - calculated by compiler
    .numConfigItems = sizeof(sysConfigData)/sizeof(Sys_Channel_Config_t),
    // Configuration data
    .dataPtr = sysConfigData
};



// This table provides a list of commands for this module. The primary purpose
// is to link each Command ID to its corresponding message handler function
const MessageRouter_CommandTableItem_t sysMessageTable[] =
{
   // {Command ID, Message Handler Function Pointer}
   { 0x01, Sys_MessageRouter_GetApplicationVersion },
   { 0x02, Sys_MessageRouter_GetProductID },
   { 0x03, Sys_MessageRouter_GetProductName },
   { 0x04, Sys_MessageRouter_Reset },
   { 0x05, Sys_MessageRouter_GetResetReason },
   { 0x06, Sys_MessageRouter_GetUptimeMillseconds }
};

const MessageRouter_Data_t sysMessageConfig =
{
 //.moduleID = SYSTEM_MODULE_ID,
 .numCommands = sizeof(sysMessageTable)/sizeof(MessageRouter_Data_t),
 .commandTable = sysMessageTable
};


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/
