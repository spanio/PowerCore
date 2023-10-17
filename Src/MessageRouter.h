/*******************************************************************************
// Core Message Router Module
*******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Incclues
// Platform Includes
#include "Platform.h"
// Other Includes
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
// Public Constant Definitions
*******************************************************************************/

/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

//-----------------------------------------------
// Response Codes
//-----------------------------------------------

/** This enumeration defines the Power response codes that may be
 * generated when processing messages.
 */
typedef enum
{
   // No error
   POWER_MESSAGEROUTER_RESPONSE_CODE_None,
   // Invalid Module ID
   POWER_MESSAGEROUTER_RESPONSE_CODE_InvalidModuleID,
   // Invalid Command ID
   POWER_MESSAGEROUTER_RESPONSE_CODE_InvalidCommandID,
   // Invalid command length
   POWER_MESSAGEROUTER_RESPONSE_CODE_InvalidCommandLength,
   // Invalid response length
   POWER_MESSAGEROUTER_RESPONSE_CODE_InvalidResponseLength,
   // Invalid Checksum
   POWER_MESSAGEROUTER_RESPONSE_CODE_InvalidChecksum,
   // Internal Error
   POWER_MESSAGEROUTER_RESPONSE_CODE_InternalError,
   // Number of Response Codes
   POWER_MESSAGEROUTER_RESPONSE_CODE_Count
} MessageRouter_ResponseCode_t;

//-----------------------------------------------
// Message Definitions
//-----------------------------------------------

// This type defines a message header
typedef struct
{
   // Unique Identifier for a destination software module
   uint16_t moduleID;

   // Identifier for a specific command in the destination software module
   uint16_t commandID;

   // Optional - Used to match responses with commands (sequencing)
   uint16_t messageID;
} MessageRouter_MessageItemHeader_t;

// This type defines the buffer for the actual message data
typedef struct
{
   // Maximum length of the data buffer (assigned by module that owns the actual buffer)
   uint16_t maxLength;

   // Length of the data that is currently in the buffer
   uint16_t length;

   // Pointer to the actual buffer
   uint16_t *data;
} MessageRouter_MessageItemBuffer_t;

// This type defines the complete Message structure common to all
// Message Router functions -- composed of Command and Response
typedef struct
{
   // Command Header
   MessageRouter_MessageItemHeader_t header;

   // Command received from external source
   MessageRouter_MessageItemBuffer_t commandParams;

   // Response built by internal module
   MessageRouter_MessageItemBuffer_t responseParams;

   // The response for this message
   MessageRouter_ResponseCode_t responseCode;
} MessageRouter_Message_t;

//-----------------------------------------------
// Command Table Definitions
//-----------------------------------------------
// This type defines a function pointer that is used in the message table for
// each module.  The handler is the destination for an incoming message.
// Command parameters (if any) are contained in the passed message structure.
// Responses (if any) are expected to be set in the passed message structure before
// the handler completes.  The message router will handle packetizing the response
// and sending the response to the sender.
typedef void (*MessageRouter_MessageHandler_t)(MessageRouter_Message_t *const message);

// This is an item in the command table for a module.  This associates a
// message handler with a command ID.
// functions of a module.
typedef struct
{
   // Command ID
   uint16_t commandID;

   // Message Handler Function
   const MessageRouter_MessageHandler_t messageHandler;
} MessageRouter_CommandTableItem_t;

//-----------------------------------------------
// Module Table Definitions
//-----------------------------------------------
// This is an item in the module table.  This associates a command table with a
// module ID.
typedef struct MessageRouter_Data_s
{
   // Module ID
   uint16_t moduleID;

   // Command Table
   const MessageRouter_CommandTableItem_t *commandTable;

   // Number of Commands in the Command Table
   uint16_t numCommands;
} MessageRouter_Data_t;

typedef struct
{

    uint32_t numConfigItems;
    const struct MessageRouter_Data_s *dataPtr;
} MessageRouter_Config_t;

//typedef MessageRouter_ConfigItem_t *MessageRouter_Config_t;

/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/*******************************************************************************
// Description:
//    Specifies the configuration tables used for the module and also specifies
//    the unique module Identifier used for routing and error reporting.
// Parameters:
//    moduleId - The numeric module identifier to be used for the module.
//       Note that this value should be unique to to each module in the system.
//    configData - Defines the required configuration data for the module.
//       Note that the data is typically defined in the provided as part of the
//       board-specific configuration files.
// Returns:
//    bool - The result of the initialization
// Return Value List:
//    true: The module was initialized successfully
//    false: The module was unable to complete successful initialization.
//    Note that calls to API functions before initialization may have unexpected results.
*******************************************************************************/
bool MessageRouter_Init(uint32_t moduleId, const MessageRouter_Config_t *configData);

/*******************************************************************************
// Description:
//    Returns the unique module identifier passed during initialization.
//    This is used if other modules want to know the Module ID for the Message Router.
// Parameters:
//    none
// Returns:
//    uint32_t - The unique module ID assigned to this module during initialization.
*******************************************************************************/
uint32_t MessageRouter_GetModuleID(void);

/** Description:
 *    This function accepts a Message Router populated with the complete
 *    Message information and a header for the Response.  This information is
 *    used to route the Message to the corresponding handler.  The handlers are
 *    expected to be define in the module's Message Table.  The Response header
 *    will be used if  the destination module wishes to respond to have the
 *    response sent back to the originating driver.
 * Parameters:
 *    message - Pointer to the Message Object to be processed
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void MessageRouter_ProcessMessage(MessageRouter_Message_t *const message);

/** Description:
 *    This function checks the given message object to verify that
 *    the command length matches the expected value.
 * Parameters:
 *    message :      Pointer to the Message Object containing the
 *                   information to be verified.
 *    commandSize :  The expected size of the command parameters in
 *                   the message object.
 * Returns:
 *    bool - The result of the size verification.
 * Return Value List:
 *    true :   The length of the command data in the given message
 *             object matches the desired length
 *    false :  The length does not match the expected value or the
 *             given message object was not valid.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
bool MessageRouter_VerifyCommandSize(MessageRouter_Message_t *const message, const uint16_t commandSize);

/** Description:
 *    This function checks the given message object to verify that
 *    a response of the specified size will fit.
 * Parameters:
 *    message :       Pointer to the Message Object containing the
 *                    information to be verified.
 *    responseSize :  The desired size for the response
 * Returns:
 *    bool - The result of the size verification.
 * Return Value List:
 *    true :   The given response size will fit in the message
 *             response
 *    false :  The given response length is too large for the
 *             message or the given message object was not valid.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
bool MessageRouter_VerifyResponseSize(MessageRouter_Message_t *const message,
                                            const uint16_t responseSize);

/** Description:
 *    This function checks the given message object to verify that no
 *    parameters were given.
 * Parameters:
 *    message - Pointer to the Message Object containing the information to be
 *    verified.
 * Returns:
 *    bool - The result of the size verification.
 * Return Value List:
 *    true - No command parameters were given in the message object
 *    false - Command parameters were sent unexpectedly for this message or the
 *    given message object was not valid.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
bool MessageRouter_VerifyNoCommandParameters(MessageRouter_Message_t *const message);

/** Description:
 *    This function checks the given message object to verify that a response
 *    with no parameters will fit. This primarily added for consistency.
 * Parameters:
 *    message - Pointer to the Message Object containing the information to be
 *    verified.
 * Returns:
 *    bool - The result of the size verification.
 * Return Value List:
 *    true - A response with no parameters will fit in the given message
 *    object.
 *    false - The given message object was not valid
 * History:
 *    * Date: Function created (EJH)    
 *
 */
bool MessageRouter_VerifyNoResponseParameters(MessageRouter_Message_t *const message);

/** Description:
 *    This function assists with verifying both the command and
 *    command and response sizes.
 * Parameters:
 *    message :       Pointer to the Message Object containing the
 *                    information to be verified.
 *    commandSize :   The expected size of the command parameters in
 *                    the message object.
 *    responseSize :  The desired size for the response
 * Returns:
 *    bool - The result of the size verification.
 * Return Value List:
 *    true :   The length of the command data in the given message
 *             object matches the desired length and the response
 *             size will fit in the message object.
 *    false :  Either the command or response size verification
 *             failed or the given message object was not valid.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
bool MessageRouter_VerifyParameterSizes(MessageRouter_Message_t *const message,
                                              const uint16_t commandSize, const uint16_t responseSize);

/** Description:
 *    This function assists with setting the size of the response data in the
 *    given message object.
 * Parameters:
 *    message - Pointer to the Message Object containing the information to be
 *    verified.
 *    responseSize - The desired size for the response
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void MessageRouter_SetResponseSize(MessageRouter_Message_t *const message, const uint16_t responseSize);

#ifdef __cplusplus
extern "C"
}
#endif
