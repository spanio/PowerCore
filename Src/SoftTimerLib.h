/*******************************************************************************
// Software Timer Libary Interface
// Provides millisecond timers based on the system timebase hardware timer. 
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
#include "Platform.h" // Compiler definitions
#include "Timebase.h" // Defines ticks
// Other Includes
#include <stdbool.h>  // Defines C99 boolean type
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

// The maximum duration for a timer. We allow for 1 day to hold prevent 
// overflow in time conversion routines
#define POWER_SOFTTIMERLIB_MAX_DURATION_DAYS       (1)
// 1 millisecond converted to microseconds
#define POWER_SOFTTIMERLIB_MICROSECONDS_PER_MILLISECOND (1000U)
// 1 second converted to milliseconds
#define POWER_SOFTTIMERLIB_MILLISECONDS_PER_SECOND (1000U)
// 1 second converted to microseconds
#define POWER_SOFTTIMERLIB_MICROSECONDS_PER_SECOND ((uint32_t)POWER_SOFTTIMERLIB_MICROSECONDS_PER_MILLISECOND * (uint32_t)POWER_SOFTTIMERLIB_MILLISECONDS_PER_SECOND)
// 1 minute converted to milliseconds
#define POWER_SOFTTIMERLIB_MILLISECONDS_PER_MINUTE ((uint32_t)POWER_SOFTTIMERLIB_MILLISECONDS_PER_SECOND * (uint32_t)60U)
// 1 hour converted to milliseconds
#define POWER_SOFTTIMERLIB_MILLISECONDS_PER_HOUR   ((uint32_t)POWER_SOFTTIMERLIB_MILLISECONDS_PER_MINUTE * (uint32_t)60U)
// 1 day converted to milliseconds
#define POWER_SOFTTIMERLIB_MILLISECONDS_PER_DAY    ((uint32_t)POWER_SOFTTIMERLIB_MILLISECONDS_PER_HOUR * (uint32_t)24U)
// Maximum timer duration converted to milliseconds.  Max uint32_t of 0xFFFFFFFF is reserved
#define POWER_SOFTTIMERLIB_MAX_DURATION_MILLISECONDS (POWER_SOFTTIMERLIB_MAX_DURATION_DAYS * POWER_SOFTTIMERLIB_MILLISECONDS_PER_DAY)
// Maximum timer duration converted to seconds
#define POWER_SOFTTIMERLIB_MAX_DURATION_SECONDS (POWER_SOFTTIMERLIB_MAX_DURATION_MILLISECONDS / POWER_SOFTTIMERLIB_MILLISECONDS_PER_SECOND)
// Maximum timer duration converted to Timebase ticks
#define POWER_SOFTTIMERLIB_MAX_DURATION_TICKS (POWER_SOFTTIMERLIB_MAX_DURATION_MILLISECONDS * TIMEBASE_DRV_NUM_TICKS_PER_MILLISECOND)


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

// Structure that represents a timer object
typedef struct
{
   // The timestamp when the timer was started
   Timebase_Tick_t startTimestamp;
   // The duration of timer in ticks
   Timebase_Tick_t durationTicks;
} SoftTimerLib_Timer_t;


/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/*******************************************************************************
// Description:
//    This function initializes the given timer structure.  The timer will be
//    marked as stopped. Since this is a library, no global state infromation
//    is stored -- each timer structure is self-contained.
// Parameters:
//    timer - A pointer to a common software timer structure.
// Returns: 
//    none 
*******************************************************************************/
void SoftTimerLib_Init(SoftTimerLib_Timer_t *const timer);

/*******************************************************************************
// Description:
//    Start a countdown timer with the specified duration. Note that the value
//    cannot be 0xFFFFFFFF since that denotes a stopped timer.  It is required
//    that the timer be periodically checked for expiration by calling
//    SoftTimerLib_IsTimerExpired faster than the SysTick will wrap.
// Parameters:
//    timer - A pointer to the common software timer structure to be started.
//    durationMilliseconds - The total number of milliseconds for timer.
*******************************************************************************/
void SoftTimerLib_StartTimer(SoftTimerLib_Timer_t *const timer,
                                   const uint32_t durationMilliseconds);


/*******************************************************************************
// Description:
//    Start a count up (measurement) timer.
//    Use SoftTimerLib_GetElapsedTimeMilliseconds or
//    SoftTimerLib_GetElapsedTimeMilliseconds to determine
//    how much time has elapsed.
// Parameters:
//    timer - A pointer to the common software timer structure to be started.
*******************************************************************************/
void SoftTimerLib_StartTimerMeasurement(SoftTimerLib_Timer_t *const timer);


/*******************************************************************************
// Description:
//    Stops the given timer.  Note that a stopped timer can never be expired.
// Parameters:
//    timer -  A pointer to the common software timer structure to be stopped.
*******************************************************************************/
void SoftTimerLib_StopTimer(SoftTimerLib_Timer_t *const timer);


/*******************************************************************************
// Description:
//    Checks to see if a given timer is running. Note that expired timers are
//    still considered to be running until SoftTimerLib_StopTimer is called.
// Parameters:
//    timer - A pointer to the common software timer structure to be checked.
// Returns:
//    bool - The current running status of the given timer
// Return Value List:
//    true - The given timer is not stopped and is still running.
//    false - The given timer is stopped.
*******************************************************************************/
bool SoftTimerLib_IsTimerRunning(SoftTimerLib_Timer_t *const timer);


/*******************************************************************************
// Description:
//    Checks to see if a given timer is expired. Note that the timer must be
//    stopped or it will continue to report that it is expired. A stopped timer
//    will never be expired.  Care must be taken to make sure the timer is
//    checked for expiration faster than the time it takes for the Timebase to
//    wrap. 
// Parameters:
//    timer - A pointer to the common software timer structure to be checked.
// Returns:
//    bool - The expiration status of the given timer
// Return Value List:
//    true - The given timer is not stopped and the duration has expiration.
//    false - The given timer is running and the duration has not expired.
*******************************************************************************/
bool SoftTimerLib_IsTimerExpired(SoftTimerLib_Timer_t *const timer);


/*******************************************************************************
// Description:
//    Returns the elapsed time for a running count up (measurement) timer in Timebase ticks.
// Parameters:
//    timer :  A pointer to the common software timer structure to be queried.
// Returns:
//    Timebase_Tick_t - The total number of ticks elapsed since the timer was started.
*******************************************************************************/
uint32_t SoftTimerLib_GetElapsedTimeTicks(SoftTimerLib_Timer_t *const timer);


/*******************************************************************************
// Description:
//    Returns the elapsed time for a running count up (measurement) timer
// Parameters:
//    timer :  A pointer to the common software timer structure to be queried.
// Returns:
//    uint32_t - The total number of milliseconds elapsed since the
//    timer was started.
*******************************************************************************/
uint32_t SoftTimerLib_GetElapsedTimeMilliseconds(SoftTimerLib_Timer_t *const timer);


/*******************************************************************************
// Description:
//    Returns the remaining time in milliseconds for a running timer.  Since a
//    stopped timer does not have remaining time, 0 will be returned.
// Parameters:
//    timer - A pointer to the common software timer structure to be queried.
// Returns:
//    Timebase_Tick_t - The remaining timer duration in Timebase ticks.
*******************************************************************************/
Timebase_Tick_t SoftTimerLib_GetRemainingTimeTicks(SoftTimerLib_Timer_t *const timer);


/*******************************************************************************
// Description:
//    Returns the remaining time in milliseconds for a running timer.  Since a
//    stopped timer does not have remaining time, 0 will be returned.
// Parameters:
//    timer - A pointer to the common software timer structure to be queried.
// Returns:
//    uint32_t - The remaining timer duration in milliseconds.
*******************************************************************************/
uint32_t SoftTimerLib_GetRemainingTimeMilliseconds(SoftTimerLib_Timer_t *const timer);



/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif

