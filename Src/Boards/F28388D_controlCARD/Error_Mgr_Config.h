/*******************************************************************************
// Error Service Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once


/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
// Other Includes


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

typedef enum {
   ERROR_MGR_ERROR_STANDARD,
   ERROR_MGR_ERROR_CRITICAL,
   ERROR_MGR_ERROR_OVER_CURRENT,
   ERROR_MGR_ERROR_OVER_VOLTAGE,
   ERROR_MGR_ERROR_OVER_TEMP,
   ERROR_MGR_ERROR_GATE_DRIVER,
   ERROR_MGR_ERROR_COUNT
} Error_Mgr_Error_t;


/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif

