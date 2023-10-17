/*******************************************************************************
// GPIO Driver - TI F2838xD Implementation
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "GPIO_Drv.h"
#include "GPIO_Drv_Config.h"
#include "GPIO_Drv_ConfigTypes.h"
// Platform Includes
#include "Port_Drv.h"
#include "Sys.h" // Delay
// Other Includes
#include "gpio.h" // TI DriverLib GPIO API
#include <stdbool.h> // Defines C99 boolean type
#include <stdint.h> // Defines C99 integer types

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
   GPIO_Drv_Config_t *gpioConfig;

   // Initialization state for the module
   bool isInitialized;

   // Enable state for the module
   bool enableState;
} GPIO_Drv_Status_t;

/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The variable used for holding all internal data for this module.
static GPIO_Drv_Status_t status;


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

// Module initiazation
bool GPIO_Drv_Init(const uint32_t moduleID, const GPIO_Drv_Config_t *configData)
{
    // Default module to uninitialized and not enabled
    status.isInitialized = false;
    status.enableState = false;

    // First, validate the given parameter is valid
    if (NULL != configData)
    {
        // Store the given configuration table
        status.gpioConfig = (GPIO_Drv_Config_t *)configData;

        // Set to initialized and configured
        status.isInitialized = true;
        status.enableState = true;
    }

    // Return initialization state
    return(status.isInitialized);
}

bool GPIO_Drv_ReadChannel(const GPIO_Drv_ChannelId_t channelId)
{
    // Default to low state
    bool pinValue = false;

    // Validate the channel
    if ((status.gpioConfig) && (status.gpioConfig->channelConfigArray) && (channelId < status.gpioConfig->numConfigItems))
    {
        // Channel is valid, get Port ID
        Port_Drv_PortId_t portId = status.gpioConfig->channelConfigArray[channelId].portId;

        // Build initial mask that is to be passed to Read register
        // Mask Write function will handle validating the pin
        uint32_t pinMask = 1UL << status.gpioConfig->channelConfigArray[channelId].pinIndex;

        // Read the pin using mask function - pin will be verified by Port Driver
        pinValue = (pinMask == (uint32_t)Port_Drv_ReadMaskedPort(portId, pinMask));
    }

    // Return the read state
    return (pinValue);
}


void GPIO_Drv_WriteChannel(const GPIO_Drv_ChannelId_t channelId, const bool activeState)
{
    // Validate the channel
    if ((status.gpioConfig) && (status.gpioConfig->channelConfigArray) && (channelId < status.gpioConfig->numConfigItems))
    {
        // Channel is valid, get Port ID
        Port_Drv_PortId_t portId = status.gpioConfig->channelConfigArray[channelId].portId;

        // Build initial mask that is to be passed to Set/Clear register
        // Mask Write function will handle validating the pin
        uint32_t pinMask = (uint32_t)(1UL << status.gpioConfig->channelConfigArray[channelId].pinIndex);

        // Determine the new level
        if (activeState)
        {
            // Set the pin using Port Driver function - pin will be verified by Port Driver
            Port_Drv_WriteMaskedPort(portId, pinMask, pinMask);
        }
        else
        {
            // Clear the pin using Port Driver function - pin will be verified by Port Driver
            Port_Drv_WriteMaskedPort(portId, pinMask, 0U);
        }
    }
}


bool GPIO_Drv_ToggleChannel(const GPIO_Drv_ChannelId_t channelId)
{
    // Default to low state
    bool updatedValue = false;

    // Validate the channel
    if ((status.gpioConfig) && (status.gpioConfig->channelConfigArray) && (channelId < status.gpioConfig->numConfigItems))
    {
        // Channel is valid, get Port ID
        Port_Drv_PortId_t portId = status.gpioConfig->channelConfigArray[channelId].portId;

        // Build initial mask that is to be passed to Set/Clear register
        // Mask Write function will handle validating the pin
        uint32_t pinMask = (uint32_t)(1UL << status.gpioConfig->channelConfigArray[channelId].pinIndex);

        // Toggle the pin - pin will be verified by Port Driver
        updatedValue = (0 != Port_Drv_ToggleMaskedPort(portId, pinMask));
    }

    return(updatedValue);
}

GPIO_Drv_PortLevel_t GPIO_Drv_ReadChannelGroup(const GPIO_Drv_GroupId_t groupId)
{
    // Default to low state
    GPIO_Drv_PortLevel_t currentLevel = 0U;

    // Validate the pointers and Group ID

    if ((status.gpioConfig) && (status.gpioConfig->channelConfigArray) && (groupId < status.gpioConfig->numGroupConfigItems))
    {
        // Channel is valid, get Port ID
        Port_Drv_PortId_t portId = status.gpioConfig->channelConfigArray[groupId].portId;

        // Read the entire port and apply group mask
        // Final value is AND of new value and Group, then shifted by the offset
        // (Level >> GroupOffset) & GroupMask
        currentLevel = Port_Drv_ReadPort(portId);
        currentLevel = (currentLevel >> status.gpioConfig->groupConfigArray[groupId].offset) & status.gpioConfig->groupConfigArray[groupId].mask;
    }

    // Return the read state
    return (currentLevel);
}


void GPIO_Drv_WriteChannelGroup(const GPIO_Drv_GroupId_t groupId, const GPIO_Drv_PortLevel_t level)
{
    // Validate the pointers and Group ID
    if ((status.gpioConfig) && (status.gpioConfig->groupConfigArray) && (groupId < status.gpioConfig->numGroupConfigItems))
    {

        // Group is valid, get Port ID
        Port_Drv_PortId_t portId = status.gpioConfig->groupConfigArray[groupId].portId;

        // Build initial mask that is to be passed to Write function
        // Final value is AND of new value and Group, then shifted by the offset
        // (NewLevel & GroupMask) << GroupOffset
        // Mask Write function will handle validating the pins
        uint32_t pinMask = (status.gpioConfig->groupConfigArray[groupId].mask) << status.gpioConfig->groupConfigArray[groupId].offset;

        // Set the pin using Port Driver function - pin will be verified by Port Driver
        // Write function will apply the mask to make sure only selected & valid pins are modified
        Port_Drv_WriteMaskedPort(portId, pinMask, level << status.gpioConfig->groupConfigArray[groupId].offset);
    }
}

GPIO_Drv_PortLevel_t GPIO_Drv_ToggleChannelGroup(const GPIO_Drv_GroupId_t groupId)
{
    // Invalid of unconfigured channels return low
    GPIO_Drv_PortLevel_t updatedLevel = 0U;

    // Validate the pointers and Group ID
    if ((status.gpioConfig) && (status.gpioConfig->groupConfigArray) && (groupId < status.gpioConfig->numGroupConfigItems))
    {
        // Group is valid, get Port ID
        Port_Drv_PortId_t portId = status.gpioConfig->groupConfigArray[groupId].portId;

        // Build initial mask that is to be passed to Write function
        // Final value is AND of new value and Group, then shifted by the offset
        // (NewLevel & GroupMask) << GroupOffset
        // Mask Write function will handle validating the pins
        uint32_t pinMask = (status.gpioConfig->groupConfigArray[groupId].mask) << status.gpioConfig->groupConfigArray[groupId].offset;

        // Set the pin using Port Driver function - pin will be verified by Port Driver
        // Write function will apply the mask to make sure only selected & valid pins are modified
        updatedLevel = Port_Drv_ToggleMaskedPort(portId, pinMask);
    }

    // Return the modified level
    return(updatedLevel);
}


void GPIO_Drv_MessageRouter_GetActiveState(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------

   typedef struct
   {
      // GPIO channel index
      uint16_t gpioChannel;
   } Command_t;

   // This structure defines the format of the response data
   typedef struct
   {
      // GPIO channel index
      uint16_t gpioChannel;
      // New enable state
      uint16_t activeState;
   } Response_t;


   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //	Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), sizeof(Response_t)))
   {
      // Cast the command buffer as the command type
      Command_t *command = (Command_t *)message->commandParams.data;
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Return the requested channel index
      response->gpioChannel = command->gpioChannel;
      // Return the current state of the given channel index
      response->activeState = GPIO_Drv_ReadChannel((GPIO_Drv_ChannelId_t)command->gpioChannel);

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}


void GPIO_Drv_MessageRouter_SetActiveState(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data
   typedef struct
   {
      // GPIO channel index
      uint16_t gpioChannel;
      // New state
      uint16_t newActiveState;
   } CommandSetGPIO_t;


   // Verify the length of the command parameters and make sure we have room for the response
   //	Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(CommandSetGPIO_t), 0))
   {
      // Cast the command buffer as the command type.
       CommandSetGPIO_t *command = (CommandSetGPIO_t *)message->commandParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      GPIO_Drv_WriteChannel((GPIO_Drv_ChannelId_t)command->gpioChannel, command->newActiveState);

      // Set the response length
      MessageRouter_SetResponseSize(message, 0);
   }
}

void GPIO_Drv_MessageRouter_ToggleActiveState(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data
   typedef struct
   {
      // GPIO channel index
      uint16_t gpioChannel;
   } Command_t;


   // Verify the length of the command parameters and make sure we have room for the response
   //	Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), 0))
   {
      // Cast the command buffer as the command type.
      Command_t *command = (Command_t *)message->commandParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      GPIO_Drv_ToggleChannel((GPIO_Drv_ChannelId_t)command->gpioChannel);

      // Set the response length
      MessageRouter_SetResponseSize(message, 0);
   }
}


