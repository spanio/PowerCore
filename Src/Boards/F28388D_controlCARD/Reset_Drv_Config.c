/*******************************************************************************
// Reset Driver Configuration Data
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "Reset_Drv.h"
#include "Reset_Drv_Config.h" // Defines configuration structure
#include "Reset_Drv_ConfigTypes.h" // Defines configuration structure
// Platform Includes
// Other Includes
#include <sysctl.h> // TI DriverLib Reset Causes

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// All reasons are cleared: Power-on reset, External reset pin, Watchdog reset, NMI watchdog reset and SCCRESETn by DCSM4
const Reset_Drv_Config_Data_t resetData[] = {

    { .resetCause = SYSCTL_CAUSE_POR }, //                 0x00000001U //!< Power-on reset
    { .resetCause = SYSCTL_CAUSE_XRS }, //                 0x00000002U //!< External reset pin
    { .resetCause = SYSCTL_CAUSE_WDRS }, //                0x00000004U //!< Watchdog reset
    { .resetCause = SYSCTL_CAUSE_NMIWDRS }, //             0x00000008U //!< NMI watchdog reset
    { .resetCause = SYSCTL_CAUSE_SCCRESET }, //            0x00000100U //!< SCCRESETn by DCSM
    { .resetCause = SYSCTL_CAUSE_HWBISTN }, //             0x00000020U //!< HWBISTn Reset
    { .resetCause = SYSCTL_CAUSE_ECAT_RESET_OUT }, //      0x00000200U //!< ECAT_RESET_OUT Reset
    { .resetCause = SYSCTL_CAUSE_SIMRESET_CPU1RSN }, //    0x00000400U //!< SIMRESET_CPU1 Reset
    { .resetCause = SYSCTL_CAUSE_SIMRESET_XRSN }, //       0x00000800U //!< SIMRESET_XRSn Reset
    { .resetCause = SYSCTL_CAUSE_CPU1RSN }, //             0x00000001U //!< Simulated CPU1Reset
};


// Common configuration structure passed to the module initialization function
extern const Reset_Drv_Config_t resetConfig =
{
     // The number of items in resetData - calculated by compiler
    .numConfigItems = sizeof(resetData)/sizeof(Reset_Drv_Config_Data_t),
    // Defines the reset configuration data passed at initialization
    .dataPtr = resetData
};

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

