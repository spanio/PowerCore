/*******************************************************************************
// Software Timer Libary Implementation
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "SoftTimerLib.h" // Module header
// Platform Includes
#include "Timebase.h" // Define System Ticks
// Other Includes
#include <stddef.h> // Defines NULL
#include <stdint.h> // C99 Integer Types


/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

// The timer duration that represents a stopped timer
#define TIMER_STOPPED_VALUE (UINT32_MAX)


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// Note: The system tick variable is defined in the SysTick driver


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

// Init the given timer
void SoftTimerLib_Init(SoftTimerLib_Timer_t *const timer)
{
   // Validate the given parameter
   if (NULL != timer)
   {
       // Set the duration to the stopped value
       timer->durationTicks = TIMER_STOPPED_VALUE;

       // Clear the starting timestamp
       timer->startTimestamp = 0U;
   }
}

// Start a countdown software timer
void SoftTimerLib_StartTimer(SoftTimerLib_Timer_t *const timer, const uint32_t durationMilliseconds)
{
   // Validate the given parameter
   if (NULL != timer)
   {
      // Make sure that the given milliseconds in in bounds
      if (durationMilliseconds <= POWER_SOFTTIMERLIB_MAX_DURATION_MILLISECONDS)
      {
         // Store the current SysTick as the start time
         timer->startTimestamp = Timebase_GetCurrentTickCount();

         // Store the duration
         timer->durationTicks = durationMilliseconds * (uint32_t)TIMEBASE_NUM_TICKS_PER_MILLISECOND;
      }
   }
}

// Start a countup software timer. This is just a timer with the max duration.
// Use SoftTimerLib_GetElapsedTimeTicks or SoftTimerLib_GetElapsedTimeMilliseconds
// to determine elasped time.
void SoftTimerLib_StartTimerMeasurement(SoftTimerLib_Timer_t *const timer)
{
   // A measurement timer is started by just starting a timer for the max duration.
   // We can then determine how much time has passed from the remaining time.
   SoftTimerLib_StartTimer(timer, POWER_SOFTTIMERLIB_MAX_DURATION_MILLISECONDS);
}

// Stop a timer (countdown or countup)
void SoftTimerLib_StopTimer(SoftTimerLib_Timer_t *const timer)
{
   // Validate the given parameter
   if (NULL != timer)
   {
      // Set the duration to the stopped value
      timer->durationTicks = TIMER_STOPPED_VALUE;
   }
}

// See if the timer is running
bool SoftTimerLib_IsTimerRunning(SoftTimerLib_Timer_t *const timer)
{
   // Timer is running if timer object is valid and tick count is not stopped value (0xFFFFFFFF)
   return((NULL != timer) && (TIMER_STOPPED_VALUE != timer->durationTicks));
}

// Determine if the timer duration has elapsed.
// Note a stopped will never be expired.
bool SoftTimerLib_IsTimerExpired(SoftTimerLib_Timer_t *const timer)
{
   // Assume timer is expired
   bool isExpired = false;

   // Validate the given parameter
   if (0 != timer)
   {
      // The basic is rule is stopped timers cannot be expired.
      // So first, we validate the timer is running.
      if (SoftTimerLib_IsTimerRunning(timer))
      {
         // Timer is running, calculate the elapsed time in ticks
         uint32_t elapsedTicks = (uint32_t)SoftTimerLib_GetElapsedTimeTicks(timer);

         // Expiration is when elapsed time has reached the timer duration
         isExpired = (elapsedTicks >= timer->durationTicks);
      }
      // else Timer is not running, so it cannot be expired
   }

   // Finally, return the result
   return(isExpired);
}

// Get the elapsed time for a timer (either countup or countdown)
Timebase_Tick_t SoftTimerLib_GetElapsedTimeTicks(SoftTimerLib_Timer_t *const timer)
{
   // Default to no time elapsed
   uint32_t elapsedTicks = 0U;

   // Check if the timer is running. A stopped timer will not have any elapsed time.
   if ((NULL != timer) && (SoftTimerLib_IsTimerRunning(timer)))
   {
       // Timer is valid and running, get the current time in ticks.

       // Determine elapsed time - Start is timer start timestamp, end is current SysTick timestamp
       // Call function to calculate elapsed time. Function will use current tick value for all calculations.
       // Function will handle if systick has wrapped as long as long as the total elapsed time does not exceed the timer maximum
       elapsedTicks = Timebase_CalculateElapsedTimeTicks(timer->startTimestamp, Timebase_GetCurrentTickCount());
   }

   // Return the elapsed time -- 0 if timer was not running or invalid
   return(elapsedTicks);
}



// Get the elapsed time in millseconds for a timer
uint32_t SoftTimerLib_GetElapsedTimeMilliseconds(SoftTimerLib_Timer_t *const timer)
{
   // Get the number of elapsed ticks 
   uint32_t elapsedTimeTicks = SoftTimerLib_GetElapsedTimeTicks(timer);

   // Return the elapsed time -- 0 if timer was not running
   return(Timebase_TicksToMilliseconds(elapsedTimeTicks));
}

// Get the remaining time in Timebase Ticks for a countdown timer
Timebase_Tick_t SoftTimerLib_GetRemainingTimeTicks(SoftTimerLib_Timer_t *const timer)
{
   Timebase_Tick_t remainingTicks = 0U;

   // Validate the given parameter
   if (NULL != timer)
   {
      // The basic is rule is stopped timers cannot be expired.
      // So first, we validate the timer is running.
      if (SoftTimerLib_IsTimerRunning(timer))
      {

         // Timer is running, calculate the elapsed time in ticks
         Timebase_Tick_t elapsedTicks = SoftTimerLib_GetElapsedTimeTicks(timer);

         // Only calculate remaining time for unexpired timers
         if (elapsedTicks < timer->durationTicks)
         {
             // Timer is not expired, calcuation remaining time
             remainingTicks = timer->durationTicks - elapsedTicks;
         }
      }
      // else not running, so remaining time is 0
   }

   // Finally, return the result
   return(remainingTicks);
}

// Get the remaining time in milliseconds for a countdown timer
uint32_t SoftTimerLib_GetRemainingTimeMilliseconds(SoftTimerLib_Timer_t *const timer)
{
   uint32_t remainingMilliseconds = 0U;

   // Validate the given parameter
   if (NULL != timer)
   {
      // Timer is valid, query thge remaining time and convert to milliseconds
      remainingMilliseconds = SoftTimerLib_GetRemainingTimeTicks(timer) / (uint32_t)TIMEBASE_NUM_TICKS_PER_MILLISECOND;
   }

   // Finally, return the result
   return(remainingMilliseconds);
}



