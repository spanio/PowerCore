/*******************************************************************************
// Scheduler Service 
// Defines the interface for the system task scheduler.
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Scheduler_Config.h" // Defines scheduled functions
// Platform Includes
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

// This is the type definition for all scheduled functions.
typedef void (*Scheduler_Function_t)(void);

// This is the structure for each scheduler entry. Interval is
// the number of scheduler ticks (milliseconds) in which this
// function is to be called. scheduledFunction is the address of
// the scheduled function. Each entry should have an Interval,
// and a pointer to the function to be called.
typedef struct
{
   // Periodic interval in which the specified function is to be called (in Milliseconds)
    // Note that this value must be less than the maximum software timer duration.
   uint32_t intervalMilliseconds;

   // The function that is to be called upon when the specified duration elaspes.
   const Scheduler_Function_t scheduledFunction;
} Scheduler_ConfigItem_t;

// Defines the structure that contains the configuration data for the module.
// This should be passed at structure to the module's initialization function.
typedef struct
{
    // The number of items defined in the schedulerConfigArray 
    uint32_t numConfigItems;

    // Forward declaration of configuration data
    // This is allows the the API functions ramain the same for all platforms but 
    // still allows each platform to customized the configuration data.
    const Scheduler_ConfigItem_t *schedulerConfigArray;
} Scheduler_Config_t;


/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/*******************************************************************************
// Description:
//    Initialize the scheduler module and configure a timer for each function
//    in the given configuration structure.  The timers are not started until
//    Scheduler_Execute() is called.
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
bool Scheduler_Init(const uint32_t moduleID, const Scheduler_Config_t *schedulerConfig);

/** Description:
 *     Entry point for the scheduler module.  Note that the
 *     function will not return until the scheduler is stopped.
 *     This function will loop through the configured schedule
 *     table and check the corresponding timer for expiration.
 *     When a timer for a scheduled item is expired, the
 *     function pointer for that scheduled item is executed.
 * Returns:
 *     none
 *
 */
void Scheduler_Execute(void);

/** Description:
 *     Stops the exectution of the task scheduler after the
 *     current task completes (if running). Note that this will
 *     not interrupt the current task.
 * Returns: 
 *     none
 */
void Scheduler_Stop(void);


/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif


