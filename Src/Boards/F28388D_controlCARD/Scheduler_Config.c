/*******************************************************************************
// Scheduler Service Configuration Data
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "Scheduler.h"
#include "Scheduler_Config.h" // Defines configuration structure
// Platform Includes
// Other Includes
#include "LED_Mgr.h"
#include "Serial.h"
#include "UART_Drv.h"

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The table defining all scheduled function and the periodic frequency to be called
const Scheduler_ConfigItem_t schedulerConfigData[] =
{
    // { ms, Pointer To Scheduled Function }
       { 100, UART_Drv_Update },
       { 100, LED_Mgr_Update },
       { 100, Serial_Update },
};


// Common configuration structure passed to the module initialization function
extern const Scheduler_Config_t schedulerConfig =
{
    // The number of items in schedulerConfigData - calculated by compiler
    // The number of items must match the definition of NUM_SCHEDULED_FUNCTIONS
    // since it is used to define internal array sizes
   .numConfigItems = sizeof(schedulerConfigData)/sizeof(Scheduler_ConfigItem_t),

    // The configuration data from the module configuration file
   .schedulerConfigArray = schedulerConfigData
};


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

