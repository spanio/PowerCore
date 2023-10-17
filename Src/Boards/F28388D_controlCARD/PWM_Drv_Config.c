/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "PWM_Drv.h"
#include "PWM_Drv_Config.h" // Defines configuration structure
#include "PWM_Drv_ConfigTypes.h" // Defines configuration structure
// Platform Includes
#include "MessageRouter.h"
// Other Includes
#include <stdlib.h> //NULL

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/


// Specifies the pwm configuration for each logical PWM channel
// Note that each channel must be specified in the same order as the defined in PWM_Drv_Channel_t
const PWM_Drv_ChannelConfig_t pwmConfigData[PWM_CHANNEL_COUNT] =
{

};

// Specified the pin configuration for each logical PWM groups
// Note that each channel must be specified in the same order as the defined in PWM_Drv_Group_t
const PWM_Drv_GroupConfig_t pwmGroupConfigData[PWM_GROUP_COUNT] =
{
    {
       // TODO
       .groupId = PWM_GROUP_COUNT,
    }
};

// Common configuration structure passed to the module initialization function
const PWM_Drv_Config_t pwmConfig =
{
    // The number of items in the pwmConfigData - calculated by compiler
    // Note that this must match PWM_DRV_CHANNEL_ID_COUNT
    .numConfigItems = sizeof(pwmConfigData) / sizeof(PWM_Drv_ChannelConfig_t),
    // PWM configuration data
    .channelConfigArray = pwmConfigData,

    // The number of items in the gpioGroupConfigData - calculated by compiler
    // Note that this must match GPIO_DRV_GROUP_ID_COUNT
    .numGroupConfigItems = sizeof(pwmGroupConfigData) / sizeof(PWM_Drv_GroupConfig_t),
    // Group configuration data
    .groupConfigArray = pwmGroupConfigData
};


// This table provides a list of commands for this module. The primary purpose
// is to link each Command ID to its corresponding message handler function
const MessageRouter_CommandTableItem_t pwmMessageTable[] =
{
   // {Command ID, Message Handler Function Pointer}
   { 1, PWM_Drv_MessageRouter_GetFrequencyHz },
   { 2, PWM_Drv_MessageRouter_SetFrequencyHz },
   { 3, PWM_Drv_MessageRouter_GetDutyCycle },
   { 4, PWM_Drv_MessageRouter_SetDutyCycle },
   { 5, PWM_Drv_MessageRouter_GetDeadtime },
   { 6, PWM_Drv_MessageRouter_SetDeadtime },
   { 7, PWM_Drv_MessageRouter_GetEnableState },
   { 8, PWM_Drv_MessageRouter_SetEnableState },
   { 9, PWM_Drv_MessageRouter_GetPhaseOffset },
   { 10, PWM_Drv_MessageRouter_SetPhaseOffset }
};


const MessageRouter_Data_t pwmMessageConfig =
{
 //.moduleID = SYSTEM_MODULE_ID,
 .numCommands = sizeof(pwmMessageTable)/sizeof(MessageRouter_CommandTableItem_t),
 .commandTable = pwmMessageTable
};



/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/
