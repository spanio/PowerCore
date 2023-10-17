/*******************************************************************************
// Reset Driver Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Reset_Drv_Config.h"
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

typedef struct Reset_Drv_Config_Data_s {
    // Reset to be cleared at startup. Once cleared,
    // another reset for the same reason can be detected, and a reset for a
    // different reason can be distinguished (instead of having two reset causes set).
    // From TI documentation: If the reset reason is used by an application, all reset causes
    // should be cleared after they are retrieved with SysCtl_getResetCause().
    uint32_t resetCause;
} Reset_Drv_Config_Data_t;


// End of C Binding Section
#ifdef __cplusplus
}
#endif

