/*******************************************************************************
// Scheduler Service
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Scheduler.h"
// Platform Includes
#include "SoftTimerLib.h"
// Other Includes
#include <stdbool.h> // Defines C99 boolean type


/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

// This structure defines the internal variables used by the module
typedef struct
{
   // Configuration Table passed at Initialization
   Scheduler_Config_t *schedulerConfig;

   // Module Id given to this module at Initialization
   uint16_t moduleId;

   // Initialization state for the scheduler module
   bool isInitialized;

   // Enable state for the scheduler module
   bool enableState;

   // The following array allocates a timer object for each scheduled item.
   SoftTimerLib_Timer_t schedulerItemTimer[MAX_SCHEDULED_FUNCTIONS];
} Scheduler_Status_t;


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The variable used for holding all internal data for this module.
static Scheduler_Status_t status;


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/** Description:
 *    Function to start the timer for a scheduled function.
 * Parameters:
 *    timerIndex - Index into the Scheduler_configTable
 * Returns:
 *    none
 */
static void StartTimer(const uint16_t timerIndex);


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

// Function to start the timer for the function at the given index
static void StartTimer(const uint16_t timerIndex)
{
   // Verify the timer index is valid
   if (timerIndex < status.schedulerConfig->numConfigItems)
   {
      // Calculate the desired interval in milliseconds and start the timer
      SoftTimerLib_StartTimer(&status.schedulerItemTimer[timerIndex], (uint32_t)status.schedulerConfig->schedulerConfigArray[timerIndex].intervalMilliseconds);
   }
}


/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

// Initialize the scheduler module
bool Scheduler_Init(const uint32_t moduleID, const Scheduler_Config_t *schedulerConfig)
{
    // Start with Scheduler disabled and uninitialized
    status.isInitialized = false;
    status.enableState = false;

    // Store the module Id for error reporting
    status.moduleId = moduleID;

    // First, validate the given parameter is valid
    if ((NULL != schedulerConfig) && (NULL != schedulerConfig->schedulerConfigArray))
    {
        // Store the given configuration table
        status.schedulerConfig = (Scheduler_Config_t *)schedulerConfig;

        // Verify the given parameter
        // Verify the number of entries in the table since that defines the number of timers
        if ((schedulerConfig) && (MAX_SCHEDULED_FUNCTIONS >= schedulerConfig->numConfigItems))
        {
            // Store the given configuration table
            status.schedulerConfig = (Scheduler_Config_t *)schedulerConfig;

            // Loop through each timer in the array and initialize
            for (uint16_t timerIndex = 0U; timerIndex < schedulerConfig->numConfigItems; timerIndex++)
            {
                SoftTimerLib_Init(&status.schedulerItemTimer[timerIndex]);
            }

            // Mark initialization complete
            status.isInitialized = true;
        }
    }

    // Return completed initialization status
    return(status.isInitialized);
}

// Scheduler entry point
void Scheduler_Execute(void)
{
   // Make sure the scheduler has been initialized
   if (status.isInitialized)
   {
       // Verify the schedule table is valid
       if (NULL != status.schedulerConfig)
       {
           // Enable the scheduler
           status.enableState = true;

           // Loop through and start all of the timers
           for (uint16_t timerIndex = 0U; timerIndex < status.schedulerConfig->numConfigItems; timerIndex++)
           {
              StartTimer(timerIndex);
           }

           // Loop through the schedule table and call the functions when they have
           // expired
           while (status.enableState)
           {
              // Loop through and check each timer for expiration
              // If expired, call the function
              for (uint16_t timerIndex = 0U; timerIndex < status.schedulerConfig->numConfigItems; timerIndex++)
              {
                 // Check the timer to see if it is expired
                 if (SoftTimerLib_IsTimerExpired(&status.schedulerItemTimer[timerIndex]))
                 {
                    // The timer is expired

                    // Restart the timer -- the time is measured from the start of the
                    // function to the start of the next time it is called
                    StartTimer(timerIndex);

                    // Verify the scheduler table has been configured and that the scheduled function is valid
                    if (status.schedulerConfig && status.schedulerConfig->schedulerConfigArray[timerIndex].scheduledFunction)
                    {
                        // Finally, call the function
                        status.schedulerConfig->schedulerConfigArray[timerIndex].scheduledFunction();
                    }
                 }
              }
           }
       }
   }
}

// Stop the scheduler
void Scheduler_Stop(void)
{
    // Just set the scheduler state to disabled
    // This will allow the while loop to exit.
    // Note that this typically would not stop operation since it would cause the watchdog to reset the device, if enabled.
    status.enableState = false;
}
