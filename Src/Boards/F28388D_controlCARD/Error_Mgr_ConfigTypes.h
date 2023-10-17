/*******************************************************************************
// Error Mgr Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Error_Mgr_Config.h"
// Platform Includes
// Other Includes
#include <stdbool.h>
#include <stdint.h> // Defines C99 integer types


/*******************************************************************************
// Start C Binding Section for C++ Compilers
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/*******************************************************************************
// Public Constant Definitions
*******************************************************************************/


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/


typedef struct Error_Mgr_Data_s {
    Error_Mgr_Error_t error;
} Error_Mgr_Data_t;


// End of C Binding Section
#ifdef __cplusplus
}
#endif

