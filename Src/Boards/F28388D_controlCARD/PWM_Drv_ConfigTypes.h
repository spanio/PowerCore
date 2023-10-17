/*******************************************************************************
// PWM Driver Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "PWM_Drv_Config.h"
// Platform Includes
// Other Includes
#include "epwm.h" // TI Driver EPWM
#include "SysCtl.h" // TI DriverLib Peripheral Clock
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

typedef struct
{
    // compModule is the Counter Compare value module (ex. EPWM_COUNTER_COMPARE_A)
    // This value is passed to EPWM_setCounterCompareValue() function
    EPWM_CounterCompareModule compModule;
    // true: Passive state = Low, Active State = High
    // false: Passive state = High, Active State = Low
   bool isActiveHigh;
   // true: invert output levels, false: default levels
   bool invert;
} PWM_Drv_OutputConfig_t;


typedef struct
{
   PWM_Drv_OutputConfig_t output1;
   PWM_Drv_OutputConfig_t output2;
   // Defines center or edge alignment mode
   //Timer_Mode_t timerMode;
   uint32_t initialPeriodHz;
   uint32_t initialDutyCycle;
   bool enableState;
} PWM_Drv_ConfigItem_t;



// TODO: Determine if Sync be included in Channel config or should it be a separate table?
// SysCtl_setSyncInputConfig(SysCtl_SyncInput syncInput, SysCtl_SyncInputSource syncSrc)
// SysCtl_SyncOutputSource syncOutputConfig;
// EPWM_SyncOutPulseMode mode;
typedef struct PWM_Drv_ChannelConfig_s
{

    // Logical channel identifier
    // Note that this is expected to match the PWM_Drv_Channel_t enumeration such that
    // the first item configuration corresponds to the the first enumerated value
    PWM_Drv_Channel_t channelId;

    // base is the base address of the EPWM module. (ex. EPWM1_BASE)
    // This value is passed to EPWM DriverLib functions.
    uint32_t base;
    // Counter Compare A settings
    PWM_Drv_OutputConfig_t outputA;
    // Counter Compare B settings
    PWM_Drv_OutputConfig_t outputB;

    // Values passed to EPWM_selectPeriodLoadEvent()
    EPWM_PeriodShadowLoadMode shadowLoadMode;
    // Value passed to EPWM_setPhaseShift
    uint16_t phaseShiftCount;

    // Initial period configured at startup for the PWM channel
    uint32_t initialPeriodHz;
    // Minimum allowed duty cycle (Float value from 0 to 1)
    // Note this must be less than maxDutyCycle
    float minDutyCycle;
    // Maximum allowed duty cycle (Float value from 0 to 1)
    // Note this must be greater than minDutyCycle
    float maxDutyCycle;
    // Default duty cycle at startup (Float value from 0 to 1)
    // A single duty cycle is configured for all configured compare outputs.
    // Note this must be between minDutyCycle and maxDutyCycle values.
    float initialDutyCycle;

    // Enable EPWM channel at startup, if true
    bool enableState;
} PWM_Drv_ChannelConfig_t;

typedef struct PWM_Drv_GroupConfig_s
{

    // Logical group identifier
    // Note that this is expected to match the PWM_Drv_Channel_t enumeration such that
    // the first item configuration corresponds to the the first enumerated value
    PWM_Drv_Group_t groupId;
} PWM_Drv_GroupConfig_t;

// End of C Binding Section
#ifdef __cplusplus
}
#endif

