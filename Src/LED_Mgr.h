/*******************************************************************************
// LED Manager
*******************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif


/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "LED_Mgr_Config.h" // Defines channel identifiers
#include "LED_Mgr_ConfigTypes.h" // Defines channel configuration structures
// Platform Includes
#include "Error_Mgr.h"
// Other Includes
#include "GPIO_Drv.h"
#include <stdint.h>


/*******************************************************************************
// Public Types
*******************************************************************************/

// This type is used to define the number of flashes for a specific error.
typedef struct
{
    // Enumerated error for this flash code
    Error_Mgr_Error_t error;
   // Flash count for this error.
   uint16_t numFlashes;
} LEDMgr_FlashCodeItem_t;

// Common configuration structure passed to the module intialization function
// Data is generally defined in the board-specific configuation file
typedef struct
{
    // The number of items in the gpioConfigData - calcuated by compiler
    // Note that this must match LED_MGR_CHANNEL_ID_COUNT
    uint16_t numConfigItems;
    // GPIO configuration data
    const LED_Mgr_ChannelConfig_t *channelConfigArray;
} LED_Mgr_Config_t;



/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/** Description:
  *    This function defines the module initialization.  The GPIO output
  *    for the LED is setup and a software timer is started.
*/
bool LED_Mgr_Init(const uint32_t moduleID, const LED_Mgr_Config_t *configData);


/** Description:
  *    The periodic function called by the schedule that determines the correct
  *    flash pattern to be displayed based on the system status.
*/
void LED_Mgr_Update(void);


/** Description:
  *    This function sets the LED output using the GPIO driver.
  * Parameters:
  *    channelId - The logical identifier of the channel to be updated.
  *    newEnableState - The desired output state for the LED.
*/
void LED_Mgr_SetLedState(const LED_Mgr_ChannelId_t channelId, const bool newEnableState);


/** Description:
  *    This function toggles the current LED state using the GPIO driver.
*/
void LED_Mgr_ToggleLedState(const LED_Mgr_ChannelId_t channelId);

void LED_Mgr_MessageRouter_GetFlashCode(MessageRouter_Message_t *const message);


#ifdef __cplusplus
extern "C"
}
#endif

