/*******************************************************************************
// Error Service Implementation
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Error_Mgr.h"
#include "Error_Mgr_Config.h"
#include "Error_Mgr_ConfigTypes.h"
// Platform Includes
#include "MessageRouter.h"
#include "Timebase.h"
// Other Includes
#include <stdbool.h> // Defines C99 boolean type
#include <stddef.h> // NULL
#include <stdint.h> // Defines C99 integer types

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

typedef struct
{
    uint32_t timestamp;
    uint16_t moduleId;
    bool state;
} Error_Mgr_SetClearDetailItem_t;

typedef struct
{
    Error_Mgr_SetClearDetailItem_t clearedDetails;
    Error_Mgr_SetClearDetailItem_t setDetails;
} Error_Mgr_SetClearDetails_t;

// This structure defines the internal variables used by the module
typedef struct
{
    // Module Id given to this module at Initialization
    uint16_t moduleId;

    // Configuration Table passed at Initialization
    const Error_Mgr_Config_t *errorConfig;

    // Initialization state for the module
    bool isInitialized;

    // Enable state for the module
    bool enableState;

    // Error flags -- currently just 32 flags
    uint32_t errorFlags;

    // Array holding details when error was last changed
    Error_Mgr_SetClearDetails_t errorDetails[ERROR_MGR_ERROR_COUNT];

    // Critical Error Mask
    uint32_t criticalErrorMask;

    // Ignore error mask
    // This mask defines a mask of errors that will not be set.
    // Note that you can always clear errors -- ignore mask is only used for setting
    // This is useful for development if you need to temporarily disable certain errors for a test
    uint32_t ignoreErrorMask;

} Error_Mgr_Status_t;


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The variable used for holding all internal data for this module.
static Error_Mgr_Status_t status;


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

// Module initialization
bool Error_Mgr_Init(const uint32_t moduleId, const Error_Mgr_Config_t *configPtr)
{
    // Default module to uninitialized and not enabled
    status.isInitialized = false;
    status.enableState = false;
    // Store the module Id for error reporting
    status.moduleId = moduleId;

    // First, validate the given parameter is valid
    if ((NULL != configPtr) && (NULL != configPtr->dataPtr))
    {
        // Store the given configuration table
        status.errorConfig = (Error_Mgr_Config_t *)configPtr;

        //-----------------------------------------------
        // Local Variable Initialization
        //-----------------------------------------------
        // Clear all errors at startup
        status.errorFlags = 0;

        // Build the bitmask of all critical errors
        for (int i = 0; i < status.errorConfig->numConfigItems; i++)
        {
            // Create mask for this error
            // The enumeration gives the index - just or into the current mask
            status.criticalErrorMask = status.criticalErrorMask | (1U << (uint32_t)(status.errorConfig->dataPtr[i].error));
        }

        // No errors excluded by default
        // Note this could be modified to initialize from NVM
        status.ignoreErrorMask = 0U;

        // Clear all error detail information
        for (int i = 0; i < ERROR_MGR_ERROR_COUNT; i++)
        {
            // Just reset data to 0 to clear
            status.errorDetails[i].clearedDetails.state = false;
            status.errorDetails[i].clearedDetails.moduleId = 0;
            status.errorDetails[i].clearedDetails.timestamp = 0;
            // Since error has not been set, just clear all values
            status.errorDetails[i].setDetails.state = false;
            status.errorDetails[i].setDetails.moduleId = 0;
            status.errorDetails[i].setDetails.timestamp = 0;
        }

        // Set to initialized and configured
        status.isInitialized = true;
        status.enableState = true;
    }

    // Return initialization state
    return(status.isInitialized);
}

bool Error_Mgr_GetErrorState(const Error_Mgr_Error_t error)
{
    // Initialize to error not set
    bool errorState = false;

    // Verify that the error does not exceed the maximum error value
    if (error < ERROR_MGR_ERROR_COUNT)
    {
        // We have a valid index, return true if the bit at that position is set
        errorState = status.errorFlags & (1U << (uint32_t)(error));
    }

    // Finally, return the error status
    return(errorState);
}

bool Error_Mgr_IsErrorEnabled(const Error_Mgr_Error_t error)
{
    // See if error (as bit mask) is set to be ignore
    // Zero result means error is not ignored (Enabled)
    // Ex. Default ignore mask 0x0 & Error Mask 0x1 => 0x0 & 0x1 => 0: Enabled
    // Non-zero result mean error is currently ignore (NOT enabled)
    // Ex. Ignore mask bit 1 0x1 & Error Mask 0x1 => 0x1 & 0x1 => 1: Disabled
    return(0U == (status.ignoreErrorMask & (1U << (uint32_t)(error))));
}

bool Error_Mgr_IsErrorCritical(const Error_Mgr_Error_t error)
{
    // Build mask of error
    uint32_t flagMask = (1U << (uint32_t)(error));
    // Compare against mask of critical errors
    // If bitwise-AND result is non-zero, then error is marked as critcial
    return(0 != (status.criticalErrorMask & flagMask));
}

void Error_Mgr_SetErrorState(const uint32_t callerModuleID, const Error_Mgr_Error_t error, const bool newState)
{
    // Verify that the error does not exceed the maximum error value
    if (error < ERROR_MGR_ERROR_COUNT)
    {
        // We have a valid index, verify that the error state is changing
        if (Error_Mgr_GetErrorState(error) != newState)
        {
            // This is a change to the previous error state, determine if we are setting or clearing the error
            if (newState)
            {
                // We are setting the error, Bitwise-OR the bit into the current flags
                // We will ignore any errors marked in the ignore error mask
                uint32_t flagMask = ~(status.ignoreErrorMask) & (1U << (uint32_t)(error));

                // If flagMask is non-zero, it is an error that can be set
                // Only currently enabled errors can trigger events
                if (0 != flagMask)
                {
                    // Store the error by bitwiser-ORing into existing flags
                    status.errorFlags = status.errorFlags | flagMask;

                    // Store the details for when error was set, cleared details will remain
                    status.errorDetails[error].setDetails.timestamp = Timebase_GetCurrentTickCount();
                    status.errorDetails[error].setDetails.moduleId = callerModuleID;
                    status.errorDetails[error].setDetails.state = newState;

                    // Determine if this is a critical error by comparing to the critical error mask
                    if (0 != (status.criticalErrorMask & flagMask))
                    {
                        // TODO - Critical Error 
                        // Triger a disable event
                    }
                }
            }
            else
            {
                // We are clearing the error, invert mask and clear bit using Bitwise-AND
                status.errorFlags = status.errorFlags & ~(1U << (uint32_t)(error));

                // Store the details for when error was cleared, set details will remain
                status.errorDetails[error].clearedDetails.timestamp = Timebase_GetCurrentTickCount();
                status.errorDetails[error].clearedDetails.moduleId = callerModuleID;
                status.errorDetails[error].clearedDetails.state = newState;
            }
        }
    }
}


void Error_Mgr_SetDetails(const Error_Mgr_Error_t error, Error_Mgr_ErrorDetailItem_t *dst, Error_Mgr_SetClearDetailItem_t *src)
{
    // Verify that the given structure is valid and that the error does not exceed the maximum error value
    if ((NULL != dst) && (NULL != src) && (error < ERROR_MGR_ERROR_COUNT))
    {
        // Given parameters are valid, copy values
        dst->state = src->state;
        dst->moduleId = src->moduleId;
        dst->timestamp = src->timestamp;
        // Age is difference between timestamp when value was changed and the current systick time
        dst->age = Timebase_CalculateElapsedTimeTicks(dst->timestamp, Timebase_GetCurrentTickCount());
    }
}

// Fetch details for the given error
void Error_Mgr_GetErrorDetails(const Error_Mgr_Error_t error, Error_Mgr_ErrorDetails_t *details)
{
    // Initialize to error not set
    bool errorState = false;

    // Verify that the given structure is valid and that the error does not exceed the maximum error value
    if ((NULL != details) && (error < ERROR_MGR_ERROR_COUNT))
    {
        // Parameters are valid, store the current error state to determine current and previous
        errorState = Error_Mgr_GetErrorState(error);

        // We separate the errors into current and previous for the user instead of the internal set/clear values
        // Determine if error is currently set or cleared
        if (errorState)
        {
            // Current condition is set condition
            Error_Mgr_SetDetails(error, &(details->current), &(status.errorDetails[error].setDetails));
            // Previous is cleared condition
            Error_Mgr_SetDetails(error, &(details->previous), &(status.errorDetails[error].clearedDetails));

        }
        else
        {
            // Current condition is cleared condition
            Error_Mgr_SetDetails(error, &(details->current), &(status.errorDetails[error].clearedDetails));
            // Previous is set condition
            Error_Mgr_SetDetails(error, &(details->previous), &(status.errorDetails[error].setDetails));
        }
        // Determine if error is set to be ignored
        details->isEnabled = Error_Mgr_IsErrorEnabled(error);
        // Determine if error is set critical
        // We return true even if error is disabled
        // A critical event must not be disabled in order to be trigger events
        details->isCritical = Error_Mgr_IsErrorCritical(error);
    }
}

bool Error_Mgr_DoAnyErrorsExist(void)
{
    // An error is present if the error flags are non-zero
    return (status.errorFlags > 0);
}

bool Error_Mgr_DoAnyCriticalErrorsExist(void)
{
    // Since the critical error mask is built at initialization, just check current flags against it
    return(status.errorFlags & status.criticalErrorMask);
}

void Error_Mgr_ClearAllErrors(const uint32_t callerModuleID)
{
    // Just loop all errors and clear each one
    for (int i = 0; i < ERROR_MGR_ERROR_COUNT; i++)
    {
        // This function will only update them if the state is changing to preserve the timestamp
        Error_Mgr_SetErrorState(callerModuleID, (Error_Mgr_Error_t)i, false);
    }
}

void Error_Mgr_EnableAllErrors(void)
{
    // Currently just clear the ignore mask to re-enable errors
    status.ignoreErrorMask = 0U;
}


/*******************************************************************************
// Message Router Function Implementations
*******************************************************************************/

// Fetch the current state of the given error
void Error_Mgr_MessageRouter_GetErrorState(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data.
   typedef struct
   {
      // Index of the error to get
      uint16_t errorIndex;
   } Command_t;

   // This structure defines the format of the response.
   typedef struct
   {
      // Error index whose state was retrieved.
      uint16_t errorIndex;
      // State of the specified error.
      uint16_t newErrorState;
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

      // Return the given error index
      response->errorIndex = command->errorIndex;

      // Fetch the error state for the given error
      response->newErrorState = (0U != Error_Mgr_GetErrorState((Error_Mgr_Error_t)response->errorIndex));


      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

// Set/Clear a the state of the given error
void Error_Mgr_MessageRouter_SetErrorState(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data.
   typedef struct
   {
      // Index of the error to set.
      uint16_t errorIndex;
      // State to set for the specified error.
      uint16_t newState;
   } Command_t;

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //	Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), 0))
   {
      // Cast the command buffer as the command type
      Command_t *command = (Command_t *)message->commandParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      Error_Mgr_SetErrorState(status.moduleId, (Error_Mgr_Error_t)command->errorIndex, (0U != command->newState));


      // Set the response length
      MessageRouter_SetResponseSize(message, 0);
   }
}

// Query if any errors exist in the system
void Error_Mgr_MessageRouter_DoErrorsExist(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the response.
   typedef struct
   {
      // Status of errors
      uint16_t errorsExist;
   } Response_t;


   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //	Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, 0, sizeof(Response_t)))
   {
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      response->errorsExist = Error_Mgr_DoAnyErrorsExist();


      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

// Clear all errors in the system
void Error_Mgr_MessageRouter_ClearAllErrors(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // No command or response params for this message.

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //	Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, 0, 0))
   {
      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      // External caller - just pass our own module Id as the callerstatus.moduleId
      Error_Mgr_ClearAllErrors(status.moduleId);

      // Set the response length
      MessageRouter_SetResponseSize(message, 0);
   }
}

// Get error flags in a bit-packed byte
void Error_Mgr_MessageRouter_GetAllErrors(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the response.
   typedef struct
   {
      // Bit-packed array of errors
      uint32_t errorFlags;
   } Response_t;


   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //	Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, 0, sizeof(Response_t)))
   {
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Simply return error flags
      response->errorFlags = status.errorFlags;

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

// Fetch the current details of the given error
void Error_Mgr_MessageRouter_GetErrorDetails(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data.
   typedef struct
   {
      // Index of the error to get
      uint16_t errorIndex;
   } Command_t;

   // This structure defines the format of the response.
   typedef struct
   {
      // Error index whose state was retrieved.
      uint16_t errorIndex;
      // Error details
      Error_Mgr_ErrorDetails_t details;
   } Response_t;

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), sizeof(Response_t)))
   {
      // Cast the command buffer as the command type
      Command_t *command = (Command_t *)message->commandParams.data;

      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Return the given error index
      response->errorIndex = command->errorIndex;

      // Fetch the error state for the given error
      Error_Mgr_GetErrorDetails((Error_Mgr_Error_t)command->errorIndex, &response->details);


      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}
