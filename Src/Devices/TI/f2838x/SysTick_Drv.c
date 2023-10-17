/*******************************************************************************
// SysTick Driver - TI F2838xD Implementation
// CPU Timer 2 is used as the System Tick timer for this platform. 
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "SysTick_Drv.h"
#include "SysTick_Drv_Config.h"
#include "SysTick_Drv_ConfigTypes.h"
// Platform Includes
// Other Includes
#include "cputimer.h" // TI CPU Timer Driver
#include <stdbool.h> // Defines C99 boolean type
#include <stdint.h> // Defines C99 integer types

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

// Define conversion factor for calculating period
#define MICROSECONDS_PER_SECOND (UINT32_C(1000000))


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

// This structure defines the internal variables used by the module
typedef struct
{
   // Configuration Table passed at Initialization
   SysTick_Drv_Config_t *systickConfig;

   // Initialization state for the systick module
   bool isInitialized;

   // Enable state for the systick module
   bool enableState;
} SysTick_Drv_Status_t;


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The variable used for holding all internal data for this module.
static SysTick_Drv_Status_t status;

// 32-bit system tick counter used for software timers
// In the current implementation, this value is read by the Timebase module
// to prevent the overhead of calling a function.
volatile SysTick_Drv_Tick_t SysTick_Drv_sysTickCount = 0U;


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

// Clear the counter used in the IRQ handler for the given timer
// Timer Base is CPUTIMER0_BASE, CPUTIMER1_BASE or CPUTIMER2_BASE
static void ResetInterruptCounter(uint32_t timerBase);

// This function initialize CPU timers to a known state.
// Timer Base is CPUTIMER0_BASE, CPUTIMER1_BASE or CPUTIMER2_BASE
static void InitCPUTimer(uint32_t timerBase);

// This function initializes the selected timer to the period specified by the
// "freq" and "period" parameters. The "freq" is entered as Hz and the period in uSeconds.
// The timer is held in the stopped state after configuration.
// Timer Base is CPUTIMER0_BASE, CPUTIMER1_BASE or CPUTIMER2_BASE
static void ConfigCPUTimer(uint32_t timerBase, float freq, float period);


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

// Reset the counters used in the interrupts
static void ResetInterruptCounter(uint32_t timerBase)
{
    // Resets interrupt counters for the three cpuTimers
    if (timerBase == CPUTIMER0_BASE)
    {
        //cpuTimer0IntCount = 0;
    }
    else if(timerBase == CPUTIMER1_BASE)
    {
        //cpuTimer1IntCount = 0;
    }
    else if(timerBase == CPUTIMER2_BASE)
    {
        // Only CPU Timer is used for system timing currently
        SysTick_Drv_sysTickCount = 0;
    }
}

// This function initialize CPU timers to a known state.
// Timer Base is CPUTIMER0_BASE, CPUTIMER1_BASE or CPUTIMER2_BASE
static void InitCPUTimer(uint32_t timerBase)
{
    // Initialize timer period to maximum
    CPUTimer_setPeriod(timerBase, 0xFFFFFFFF);

    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
    CPUTimer_setPreScaler(timerBase, 0);

    // Make sure timer is stopped
    CPUTimer_stopTimer(timerBase);

    // Reload all counter register with period value
    CPUTimer_reloadTimerCounter(timerBase);

    // Reset interrupt counters for the cpuTimer
    ResetInterruptCounter(timerBase);
}

// This function initializes the selected timer to the period specified by the
// "freq" and "period" parameters. The "freq" is entered as Hz and the period in uSeconds.
// The timer is held in the stopped state after configuration.
// Timer Base is CPUTIMER0_BASE, CPUTIMER1_BASE or CPUTIMER2_BASE
static void ConfigCPUTimer(uint32_t timerBase, float freq, float period)
{
    uint32_t temp;

    // Initialize timer period
    temp = (uint32_t)(freq / MICROSECONDS_PER_SECOND * period);
    CPUTimer_setPeriod(timerBase, temp);

    // Set pre-scale counter to divide by 1 (SYSCLKOUT)
    CPUTimer_setPreScaler(timerBase, 0);

    // Initializes timer control register. The timer is stopped, reloaded,
    // free run disabled, and interrupt enabled.
    // Additionally, the free and soft bits are set
    CPUTimer_stopTimer(timerBase);
    CPUTimer_reloadTimerCounter(timerBase);
    CPUTimer_setEmulationMode(timerBase, CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
    CPUTimer_enableInterrupt(timerBase);

    // Reset interrupt counters for the cpuTimer
    ResetInterruptCounter(timerBase);
}


/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

bool SysTick_Drv_Init(const uint32_t moduleID, const SysTick_Drv_Config_t *configData)
{
    // Start with SysTick disabled and uninitialized
    status.isInitialized = false;
    status.enableState = false;

    // Verify the given parameter
    if (configData)
    {
        // Store the given configuration table
        status.systickConfig = (SysTick_Drv_Config_t *)configData;

        // Loop through each timer in the array and initialize
        for (uint16_t i = 0U; i < status.systickConfig->numConfigItems; i++)
        {
            // Store the channel data for easy access
            SysTick_Drv_Channel_Config_t *channelData = (SysTick_Drv_Channel_Config_t *)(&(configData->dataPtr[i]));

            // Verify the Channel Id matches
            if (i == channelData->channelId)
            {
                // Verify the callback is valid
                if (channelData->callback)
                {
                    // TODO - Currently only CPU Timer 2 is implemented
                     SysCtl_enablePeripheral(channelData->peripheral);

                    // Register ISRs for each CPU Timer interrupt
                    Interrupt_register(channelData->interruptNumber, channelData->callback);

                    // Initialize the configured Device CPU Timers
                    InitCPUTimer(channelData->timerBase);

                    // Configure CPU Timebase Frequency and Period (us)
                    ConfigCPUTimer(channelData->timerBase, SYS_SYSCLK_FREQ, SYSTICK_DRV_NUM_TICKS_PER_SECOND);

                    // Initialize the global variable used for counting the SysTick
                    SysTick_Drv_sysTickCount = 0U;

                    // Mark initialization complete if at least one channel was configured
                    status.isInitialized = true;
                }
            }
        }
    }

    // Return the result of the initialization
    return(status.isInitialized);
}

void SysTick_Drv_SetEnableState(bool enableState)
{
    // Verify the given parameter
    if (status.systickConfig)
    {
        // Loop through each timer in the array and initialize
        for (uint16_t i = 0U; i < status.systickConfig->numConfigItems; i++)
        {
            // Store the channel data for easy access
            SysTick_Drv_Channel_Config_t *channelData = (SysTick_Drv_Channel_Config_t *)(&(status.systickConfig->dataPtr[i]));

            // Verify the Channel Id matches
            if (i == channelData->channelId)
            {
                // See if we are enabling or disabling
                if (enableState)
                {
                    // Enable CPU Timer IRQ used for Timebase
                    // To ensure precise timing, use write-only instructions to write to the
                    // entire register.
                    CPUTimer_enableInterrupt(channelData->timerBase);

                    // Enables CPU int14 connected to CPU-Timer 2
                    Interrupt_enable(channelData->interruptNumber);

                    // Starts CPU Timer 2 used for Timebase
                    CPUTimer_startTimer(channelData->timerBase);

                    // Initialize the global variable used for counting the SysTick
                    SysTick_Drv_sysTickCount = 0U;

                    // Mark initialization complete if at least one channel was configured
                    status.enableState = true;
                }
                else
                {
                    // Mark initialization complete if at least one channel was configured
                    status.enableState = false;

                    // Stop CPU Timer 2 used for Timebase
                    CPUTimer_stopTimer(channelData->timerBase);

                    // Enables CPU int14 connected to CPU-Timer 2
                    Interrupt_disable(channelData->interruptNumber);

                    // Disbale CPU Timer IRQ used for Timebase
                    CPUTimer_disableInterrupt(channelData->timerBase);
                }
            }
        }
    }
}


/*******************************************************************************
// Interrupt Handler
*******************************************************************************/

// IRQ for CPU Timer 2 - Used for System Tick (cpuTimer2ISR)
INTERRUPT_FUNC void SysTick_Handler(void)
{
   // Simply increment the SysTick counter by 1
   SysTick_Drv_sysTickCount++;
}
