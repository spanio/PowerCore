/*******************************************************************************
// LED Service
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "LED_Mgr.h"
#include "LED_Mgr_Config.h"
#include "LED_Mgr_ConfigTypes.h"
// Platform Includes
#include "Error_Mgr_Config.h"
#include "GPIO_Drv.h"
#include "SoftTimerLib.h"
// Other Includes
#include "Control.h"
#include <stdint.h> // Defines C99 integer types
#include <stdbool.h> // Defines C99 boolean type


/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

// This is the "Off" state blink rate
#define LED_BLINK_TIME_OFF_MS    (100)
// This is the pause at the end of flashing an error code
#define LED_BLINK_TIME_OFF_PAUSE_MS (2000)
// This is the "Idle" state blink rate
#define LED_BLINK_TIME_IDLE_MS    (100)
// This defines the length of the pause after displaying a flash code
#define LED_IDLE_END_OF_SEQUENCE_PAUSE_MS  (2000)
// This is the LED blink rate if device is running
#define LED_BLINK_TIME_RUNNING_MS   (250)
// This is the "error" blink rate used to display flash codes
#define LED_BLINK_TIME_ERROR_MS     (250)
// This defines the length of the pause after displaying a flash code
#define LED_ERROR_END_OF_SEQUENCE_PAUSE_MS  (2000)
// The total number of flash code entries in the configuration table
#define LED_NUM_FLASH_CODES (sizeof(ledMgrFlashCodeTable)/sizeof(LEDMgr_FlashCodeItem_t))


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/



// This structure defines the internal variables used by the module
typedef struct
{
    // Configuration Table passed at Initialization
    LED_Mgr_Config_t *ledConfig;

    // Initialization state for the module
    bool isInitialized;

    // Enable state for the module
    bool enableState;

   // Target number of flashes to display.
   uint16_t targetErrorCodeFlashes;

   // Counter used for displaying the proper number of error flashes.
   uint16_t currentFlashCount;

   // Variable that tracks the current LED output state
   bool currentLEDState[LED_MGR_CHANNEL_ID_COUNT];

   // Timer used to blink the led at the desired rate
   SoftTimerLib_Timer_t timer;
} LED_Status_t;


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

static const LEDMgr_FlashCodeItem_t ledMgrFlashCodeTable[] =
{
   // Error, Number of Flashes
   // {NOTE_1_BLINK_NOT_USED, 1}
   { ERROR_MGR_ERROR_CRITICAL,   2 },
   // TODO - Placeholders for feedback errors
   { ERROR_MGR_ERROR_OVER_CURRENT,   3 },
   { ERROR_MGR_ERROR_OVER_VOLTAGE,   3 },
   { ERROR_MGR_ERROR_OVER_TEMP,      3 },
};


// The variable used for holding all internal data for this module.
static LED_Status_t status;


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/** Description:
  *    This function scans through the flash code table to see if any of the
  *    configured errors are set.  Note that only the first flash code located
  *    is returned.
  * Returns:
  *    uint16_t - The located flash code that should begin being
  *    shown.  If no code is found, 0 is returned.
  * Return Value List:
  *    0 - No flash code is was found. Blink normally.
  *    1 - Reserved since it is hard to distinguish between a normal blink.
  *    2+ - The desired number of flashes to be shown.
*/
static uint16_t CheckForFlashCode(void);


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


// Check for a new flash code
static uint16_t CheckForFlashCode(void)
{
   // Initialize the flash code code to none
   uint16_t locatedFlashCode = 0U;

   // Start at the first table entry
   uint16_t tableIndex = 0U;

   // Loop through until the end of the table or we find a flash code
   while ((tableIndex < LED_NUM_FLASH_CODES) && (locatedFlashCode == 0))
   {
      // See if the error for the current index is set
      if (Error_Mgr_GetErrorState(ledMgrFlashCodeTable[tableIndex].error))
      {
         // We found an error, store the flash count and exit
         locatedFlashCode = ledMgrFlashCodeTable[tableIndex].numFlashes;
      }

      // Move to the next index
      tableIndex++;
   }

   // Return the number of flashes, if any
   return(locatedFlashCode);
}

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

// Module initiazation
bool LED_Mgr_Init(const uint32_t moduleID, const LED_Mgr_Config_t *configData)
{
    // Default module to uninitialized and not enabled
    status.isInitialized = false;
    status.enableState = false;

    // First, validate the given parameter is valid
    if (NULL != configData)
    {
        // Store the given configuration table
        status.ledConfig = (LED_Mgr_Config_t *)configData;

        //-----------------------------------------------
        // Local Variable Initialization
        //-----------------------------------------------
        // No error flashes by default
        status.targetErrorCodeFlashes = 0;
        // error flash count starts at zero.
        status.currentFlashCount = 0;
        // Initialize the timer
        SoftTimerLib_Init(&status.timer);

        // Make sure LED is off to begin with
        // Read the initial state of LEDs
        status.currentLEDState[LED_MGR_CHANNEL_ID_FAULT] = GPIO_Drv_ReadChannel(status.ledConfig->channelConfigArray[LED_MGR_CHANNEL_ID_FAULT].configuration.gpioChannelId);
        status.currentLEDState[LED_MGR_CHANNEL_ID_STATUS] = GPIO_Drv_ReadChannel(status.ledConfig->channelConfigArray[LED_MGR_CHANNEL_ID_STATUS].configuration.gpioChannelId);

        // Start the LED at the normal blink rate
        SoftTimerLib_StartTimer(&status.timer, (uint32_t)LED_BLINK_TIME_IDLE_MS);

        // Set to initialized and configured
        status.isInitialized = true;
        status.enableState = true;
    }

    // Return initialization state
    return(status.isInitialized);
}

// Update the LED state using the GPIO Driver
void LED_Mgr_SetLedState(const LED_Mgr_ChannelId_t channelId, const bool newEnableState)
{
    // Init the output to low
    bool outputLevel = false;

    // Verify the channelId
    if (channelId < LED_MGR_CHANNEL_ID_COUNT)
    {
        // Channel is valid, dee if we are changing the state
       if (newEnableState != status.currentLEDState[channelId])
       {
          // State is changing, update it
          // Note that the GPIO driver will handle translation for active low
           // AL & True  = RESET
           // AH & True  = SET
           // AL & False = SET
           // AH & False = RESET
           if (((status.ledConfig->channelConfigArray[channelId].configuration.isActiveHigh) && (newEnableState)) || ((!status.ledConfig->channelConfigArray[channelId].configuration.isActiveHigh) && (!newEnableState)))
           {
              // Set output to high
              outputLevel = true;
           }
           // else output low value
          GPIO_Drv_WriteChannel(status.ledConfig->channelConfigArray[channelId].configuration.gpioChannelId, outputLevel);
          // Store the new state
          status.currentLEDState[channelId] = newEnableState;
       }
       // Else, no change
    }

}

// Toggle the Fault LED State
void LED_Mgr_ToggleLedState(const LED_Mgr_ChannelId_t channelId)
{
   // Toggle based on the current state
   LED_Mgr_SetLedState(channelId, !status.currentLEDState[channelId]);
}

// Scheduled function for updating the LED
void LED_Mgr_Update(void)
{
    // Blink Rate depending on current state in ms
   uint32_t blinkTimeMs = (uint32_t)LED_BLINK_TIME_IDLE_MS;

   //-----------------------------------------------
   // Error Code Blink
   //-----------------------------------------------
   if (status.targetErrorCodeFlashes > 0)
   {
      // There is an error code to display, increment the flash count on the
      // rising edge of the flash (LED on) and start the pause on the falling
      // edge (LED off) of the last pulse.

       // When in fault, disable status LED
       LED_Mgr_SetLedState(LED_MGR_CHANNEL_ID_STATUS, false);

      // See if the timer has expired
      if (SoftTimerLib_IsTimerExpired(&status.timer))
      {
         // Timer has expired, so toggle the LED
         LED_Mgr_ToggleLedState(LED_MGR_CHANNEL_ID_FAULT);

         // Initialize the timer up for the error flash time.
         blinkTimeMs = (uint32_t)LED_BLINK_TIME_ERROR_MS;

         // Do we still have more flashes to display?
         if (status.currentFlashCount < status.targetErrorCodeFlashes)
         {
            // We have not output all flashes yet, increment the flash count
            // each time the LED is turned on.
            if (status.currentLEDState[LED_MGR_CHANNEL_ID_FAULT])
            {
               status.currentFlashCount++;
            }
         }
         else
         {
            // We have output all of the LED flashes, but we must wait for the
            // LED to turn off so we get a complete last flash.  Once the LED
            // turns off, reset the flash count and then pause to make the
            // flash code discernible.
            if (!status.currentLEDState[LED_MGR_CHANNEL_ID_FAULT])
            {
               // Reset the flash count
               status.currentFlashCount = 0;

               // Over-ride the timer for the end of the sequence pause.
               blinkTimeMs = (uint32_t)LED_ERROR_END_OF_SEQUENCE_PAUSE_MS;

               // Check for new errors in-case things have changed since we
               // started blinking the current error code
               status.targetErrorCodeFlashes = CheckForFlashCode();
            }
         }

         // Restart the timer with the appropriate timing
         SoftTimerLib_StartTimer(&status.timer, blinkTimeMs);
      }
   }
   //-----------------------------------------------
   // Normal LED Blink
   //-----------------------------------------------
   else
   {
      // Check to see if any exist now
      status.targetErrorCodeFlashes = CheckForFlashCode();

      // Control disabled -- slow blink
      if (status.currentLEDState[LED_MGR_CHANNEL_ID_STATUS])
      {
          // If on currently, off time will be the the end of sequence time
          Control_State_t currentState = Control_GetState();
          if ((CONTROL_STATE_RUNNING == currentState) || (CONTROL_STATE_THROTTLED == currentState))
          {
              // If runing, use same off time as the on time
              blinkTimeMs = LED_BLINK_TIME_IDLE_MS;
          }
          else
          {
              // If not a running state, use long pause between flashes
              blinkTimeMs = LED_IDLE_END_OF_SEQUENCE_PAUSE_MS;
          }
      }
      else
      {
          // If off current, we'll use the idle blink time
          blinkTimeMs = (uint32_t)LED_BLINK_TIME_IDLE_MS;
      }

      // See if the timer has expired
      if (SoftTimerLib_IsTimerExpired(&status.timer))
      {
         // Timer has expired, so toggle the LED
         LED_Mgr_ToggleLedState(LED_MGR_CHANNEL_ID_STATUS);
         // When blinking normally, disable fault LED
         LED_Mgr_SetLedState(LED_MGR_CHANNEL_ID_FAULT, false);

         // Restart the timer with the normal blink rate
         SoftTimerLib_StartTimer(&status.timer, blinkTimeMs);
      }
   }
}

/*******************************************************************************
// Message Router Function Implementations
*******************************************************************************/


// Get flash code in progress
void LED_Mgr_MessageRouter_GetFlashCode(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the response.
   typedef struct
   {
      // Flash code currently being displayed.
      uint16_t flashCode;
   } Response_t;

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, 0, sizeof(Response_t)))
   {
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Return the requested channel index
      response->flashCode = status.targetErrorCodeFlashes;

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}
