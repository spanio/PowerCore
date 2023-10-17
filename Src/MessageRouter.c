/*******************************************************************************
// Core Message Router Module
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Platform Includes
#include "MessageRouter.h"
// Other Includes
#include <limits.h> // Defines number of bits in a char
#include <stdbool.h>
#include <stdlib.h> // NULL
#include <stdint.h>

/*******************************************************************************
// Constants
*******************************************************************************/

// The total number of modules define in the Message Router configuration
//#define NUM_MESSAGEROUTER_MODULES (sizeof(messageRouterConfigTable) / sizeof(MessageRouter_ConfigItem_t))
#define NUM_MESSAGEROUTER_MODULES (0)

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

typedef struct
{
    // Module Id given to this module at Initialization
    uint16_t moduleId;

    // Initialization state for the module
    bool isInitialized;

    // Enable state for the module
    bool enableState;

    // Configuration data for the module provide passed at Init
    const MessageRouter_Config_t *messageRouterTable;
} MessageRouter_Status_t;

/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

static MessageRouter_Status_t status;


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

bool MessageRouter_Init(uint32_t moduleId, const MessageRouter_Config_t *configData)
{
    // Default module to uninitialized and not enabled
    status.isInitialized = false;
    status.enableState = false;
    // Store the module Id for error reporting
    status.moduleId = moduleId;

    // First, validate the given parameter is valid
    if (NULL != configData)
    {
        // Store the given configuration table
        status.messageRouterTable = (MessageRouter_Config_t *)configData;

        // Set to initialized and configured
        status.isInitialized = true;
        status.enableState = true;
    }

    // Return initialization state
    return(status.isInitialized);

}

// Returns the module id set at initialization
uint32_t MessageRouter_GetModuleID(void)
{
    // Simply return the module ID given at initialization
    return(status.moduleId);
}

// Lookup and execute the corresponding handler for a message
void MessageRouter_ProcessMessage(MessageRouter_Message_t *const message)
{
   // Always initialize the length of the response buffer to zero
   message->responseParams.length = 0;

   // Assume module not found
   message->responseCode = POWER_MESSAGEROUTER_RESPONSE_CODE_InvalidModuleID;

   // Loop through the module table and find the correct module ID
   for (uint32_t i = 0U; i < status.messageRouterTable->numConfigItems; i++)
   {
      // See if the module ID is found in the table
      if (status.messageRouterTable->dataPtr[i].moduleID == message->header.moduleID)
      {
         // Module was valid, assume command not found
         message->responseCode = POWER_MESSAGEROUTER_RESPONSE_CODE_InvalidCommandID;

         // Loop through the table and find the correct command ID
         for (uint16_t j = 0U; j < status.messageRouterTable->dataPtr[i].numCommands; j++)
         {
            // See if the command ID is found in the table
            if (status.messageRouterTable->dataPtr[i].commandTable[j].commandID == message->header.commandID)
            {
               // Command ID found, note that the message is valid up to this point
               message->responseCode = POWER_MESSAGEROUTER_RESPONSE_CODE_None;

               // Send the message to the massage handler, if it is not NULL
               if (status.messageRouterTable->dataPtr[i].commandTable[j].messageHandler != 0)
               {
                  // Function is not NULL, so call it
                   status.messageRouterTable->dataPtr[i].commandTable[j].messageHandler(message);
               }

               // Exit the command loop
               break;
            }
         }

         // Exit the module loop
         break;
      }
   }
}


// Verify the expected size of the command parameters
bool MessageRouter_VerifyCommandSize(MessageRouter_Message_t *const message, const uint16_t commandSize)
{
   bool isValid = false;

   // Verify the message pointer
   if (message != 0)
   {
      // Message is valid, verify that the command parameters are the expected size
      if (message->commandParams.length == commandSize)
      {
         // Parameter size matches
         isValid = true;
      }
      else
      {
         // The command buffer does not match the expected value, set the response code.
         message->responseCode = POWER_MESSAGEROUTER_RESPONSE_CODE_InvalidCommandLength;
      }
   }

   return(isValid);
}


// Verify the given command parameter size is 0
bool MessageRouter_VerifyNoCommandParameters(MessageRouter_Message_t *const message)
{
   return(MessageRouter_VerifyCommandSize(message, 0));
}


// Verify that a Response with the given size can be placed in the message object
bool MessageRouter_VerifyResponseSize(MessageRouter_Message_t *const message, const uint16_t responseSize)
{
   bool isValid = false;

   // Verify the message pointer is valid
   if (message != 0)
   {
      // Verify that the response will fit
      if (responseSize <= message->responseParams.maxLength)
      {
         // Response size fits
         isValid = true;
      }
      else
      {
         // The response buffer is too small, set the response code.
         message->responseCode = POWER_MESSAGEROUTER_RESPONSE_CODE_InvalidResponseLength;
      }
   }

   return(isValid);
}


// Verify that a Response with no parameters can be placed in the message object
bool MessageRouter_VerifyNoResponseParameters(MessageRouter_Message_t *const message)
{
   return(MessageRouter_VerifyResponseSize(message, 0));
}


// Verify both command and response sizes match expected values
bool MessageRouter_VerifyParameterSizes(MessageRouter_Message_t *const message, const uint16_t commandSize,
                                              const uint16_t responseSize)
{
   // Default to failed
   bool wasSuccessful = false;

   // Verify the message pointer is valid
   if (message != 0)
   {
      // Verify the length of the command (error response set, if necessary)
#if (16 == CHAR_BIT)
      if (MessageRouter_VerifyCommandSize(message, 2 * commandSize))
#else
          if (MessageRouter_VerifyCommandSize(message, commandSize))
#endif
      {
         // Verify that the response will fit  (error response set, if necessary)
         if (MessageRouter_VerifyResponseSize(message, responseSize))
         {
            // Success
            wasSuccessful = true;
         }
      }
   }

   // Return the success result
   return(wasSuccessful);
}


// Set the response length
void MessageRouter_SetResponseSize(MessageRouter_Message_t *const message, const uint16_t responseSize)
{
   // Verify the message pointer is valid
   if (message != 0)
   {
#if (16 == CHAR_BIT)
      // Simply set the length, note that it is expected the buffer has been checked before processing the message
      message->responseParams.length = 2 * responseSize;
#else
      message->responseParams.length = responseSize;
#endif
   }
}
