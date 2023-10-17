/*******************************************************************************
// Timebase Service Implementation
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Timebase.h" // Module header
// Platform Includes
#include "SysTick_Drv.h" // SysTick counter value
// Other Includes


/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

extern uint32_t Timer_Freq;

#define TIMER_TICK2MS( _x )		((_x) / (Timer_Freq/1000000uL))
#define TIMER_TICK2NS( _x )     (((_x)*1000) / (Timer_Freq/1000000uL))
#define TIMER_MS2TICK( _x )     ((uint32_t)(((uint32_t)Timer_Freq * (_x) )/UINT32_C(1000)))
#define TIMER_S2MS( _x )        ((_x) * 1000UL)

// Define conversion factor for calculating period
#define MICROSECONDS_PER_SECOND (UINT32_C(1000000))

// Configure system to use 1000 ticks per second (each tick = 1ms)
#define SYSTICK_DRV_NUM_TICKS_PER_SECOND (UINT32_C(1000))
#define TIMEBASE_NUM_TICKS_PER_MILLISECOND (SYSTICK_DRV_NUM_TICKS_PER_SECOND/UINT32_C(1000))

// 200MHz SYSCLK frequency computed based on the above SYS_SETCLOCK_CFG
//#define SYS_SYSCLK_FREQ          ((SYS_OSCSRC_FREQ * PLL_INTEGER_MULTIPLIER * PLL_FRACTIONAL_MULTIPLIER) / PLL_SYSCLK_DIVIDER)


typedef uint32_t TimerTick;


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

// Module intialization
bool Timebase_Init(const uint32_t moduleID, const void *configData)
{
    // Nothing to do since we just wrap the System Tick Driver
    // Return success
    return (true);
}

// Returns current Timebase Tick value used for system timing
Timebase_Tick_t Timebase_GetCurrentTickCount(void)
{
   // Simply return the current 32-bit SysTick value
   // There isn't really a need to add an extra function call to SysTick_Drv_GetCurrentTickCount
   return((Timebase_Tick_t)SysTick_Drv_sysTickCount);
}

// Convert a tick count into milliseconds
uint32_t Timebase_TicksToMilliseconds(Timebase_Tick_t const tickCount)
{
   // Return the converted time -
   return((uint32_t)tickCount / (uint32_t)TIMEBASE_NUM_TICKS_PER_MILLISECOND);
}

// Get the elapsed time for two tick timestamps (end - start)
// This function will adjust for wrapping (If end < start)
Timebase_Tick_t Timebase_CalculateElapsedTimeTicks(const Timebase_Tick_t startTickCount, const Timebase_Tick_t endTickCount)
{
    // Default to no time elapsed
    Timebase_Tick_t elapsedTicks = 0U;

    // Determine elapsed time
     // First we have to check to see if the tick count has wrapped
     if (endTickCount >= startTickCount)
     {
        // SysTick has not wrapped, just subtract
        elapsedTicks = (endTickCount - startTickCount);
     }
     else
     {
        // The SysTick has wrapped, determine how much time was left before wrap and
        // add to current time
        elapsedTicks = endTickCount + (1U + (uint32_t)TIMEBASE_MAX_TICK_VALUE - startTickCount);
     }

     // Return the result of the calculation
     return(elapsedTicks);
}


