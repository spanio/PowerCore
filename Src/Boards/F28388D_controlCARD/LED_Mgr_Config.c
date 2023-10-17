/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "LED_Mgr.h"
#include "LED_Mgr_Config.h"
#include "LED_Mgr_ConfigTypes.h" // Defines configuration structure
// Platform Includes
#include "GPIO_Device.h"
// Other Includes
#include "gpio.h" // TI DriverLib Ports
#include "pin_map.h" // TI DriverLib Pin Mapping


/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The configuration for each feedback led used by the system
// Note that the number of entries must match LED_MGR_CHANNEL_ID_COUNT
const LED_Mgr_ChannelConfig_t ledConfigData[LED_MGR_CHANNEL_ID_COUNT] =
{
// STATUS
   {
      .ledChannelId = LED_MGR_CHANNEL_ID_STATUS,
      .configuration = {
         .gpioChannelId = GPIO_DRV_CHANNEL_ID_LED_STATUS,
         .isActiveHigh = true

      }
   },
   //FAULTLED_Mgr_Data_t
   {
      .ledChannelId = LED_MGR_CHANNEL_ID_FAULT,
      .configuration = {
         .gpioChannelId = GPIO_DRV_CHANNEL_ID_LED_FAULT,
         .isActiveHigh = true

      }
   }
};



// Common configuration structure passed to the module intialization function
const LED_Mgr_Config_t ledConfig =
{

   // The number of items in the ledConfigData - calcuated by compiler
   // Note that this must match LED_MGR_CHANNEL_ID_COUNT
   .numConfigItems = sizeof(ledConfigData) / sizeof(LED_Mgr_ChannelConfig_t),
   // LED configuration data
   .channelConfigArray = ledConfigData
};

// This table provides a list of commands for this module.
const MessageRouter_CommandTableItem_t ledMessageTable[] =
{
   // {Command ID, Message Handler Function Pointer}
   { 1, LED_Mgr_MessageRouter_GetFlashCode },
};

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


/*******************************************************************************
// Public Function Implementations
*******************************************************************************/


