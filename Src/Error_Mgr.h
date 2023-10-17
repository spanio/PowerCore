/*******************************************************************************
// Error Manager
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
#include "Error_Mgr_Config.h" // Defines channel identifiers
// Platform Includes
#include "MessageRouter.h"
// Other Includes
#include <stdint.h>


/*******************************************************************************
// Public Types
*******************************************************************************/

// Structure used to provide additional details on when an error was last set
typedef struct
{
    uint32_t timestamp;
    uint32_t age;
    uint16_t moduleId;
    bool state;
} Error_Mgr_ErrorDetailItem_t;

typedef struct
{
    Error_Mgr_ErrorDetailItem_t current;
    Error_Mgr_ErrorDetailItem_t previous;
    bool isEnabled;
    bool isCritical;
} Error_Mgr_ErrorDetails_t;

// Common configuration structure passed to the module initialization function
// Data is generally defined in the board-specific configuration file
typedef struct
{
    // The number of items in the errorConfigData - calculated by compiler
    uint32_t numConfigItems;
    // Configuration data
    const struct Error_Mgr_Data_s *dataPtr;
} Error_Mgr_Config_t;


/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

bool Error_Mgr_Init(const uint32_t moduleId, const Error_Mgr_Config_t *configPtr);
void Error_Mgr_SetErrorState(const uint32_t moduleID, const Error_Mgr_Error_t error, const bool newState);
bool Error_Mgr_GetErrorState(const Error_Mgr_Error_t error);
void Error_Mgr_GetErrorDetails(const Error_Mgr_Error_t error, Error_Mgr_ErrorDetails_t *details);
bool Error_Mgr_DoAnyErrorsExist(void);
bool Error_Mgr_DoAnyCriticalErrorsExist(void);
void Error_Mgr_ClearAllErrors(const uint32_t callerModuleID);
void Error_Mgr_MessageRouter_GetErrorState(MessageRouter_Message_t *const message);
void Error_Mgr_MessageRouter_SetErrorState(MessageRouter_Message_t *const message);
void Error_Mgr_MessageRouter_DoErrorsExist(MessageRouter_Message_t *const message);
void Error_Mgr_MessageRouter_ClearAllErrors(MessageRouter_Message_t *const message);
void Error_Mgr_MessageRouter_GetAllErrors(MessageRouter_Message_t *const message);
void Error_Mgr_MessageRouter_GetErrorDetails(MessageRouter_Message_t *const message);

#ifdef __cplusplus
extern "C"
}
#endif

