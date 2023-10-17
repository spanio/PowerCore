/*******************************************************************************
// SysTick Driver Configuration Data
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "SysTick_Drv.h"
#include "SysTick_Drv_Config.h" // Defines configuration structure
#include "SysTick_Drv_ConfigTypes.h" // Defines configuration structure
// Platform Includes
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

const SysTick_Drv_Channel_Config_t sysTickData[SYSTICK_DRV_CHANNEL_ID_COUNT] = {

    {
     .channelId = SYSTICK_DRV_CHANNEL_ID_TIMER2,
     .timerBase = CPUTIMER2_BASE,
     .peripheral = SYSCTL_PERIPH_CLK_TIMER2,
     .interruptNumber = INT_TIMER2,
     .callback = SysTick_Handler
    }
};


// Common configuration structure passed to the module initialization function
extern const SysTick_Drv_Config_t sysTickConfig =
{
     // The number of items in the systickData - calculated by compiler
     // Note that this must match SYSTICK_DRV_CHANNEL_ID_COUNT
    .numConfigItems = sizeof(sysTickData)/sizeof(SysTick_Drv_Channel_Config_t),
    // Defines the list of CPU Timers used by system
    .dataPtr = sysTickData
};

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

