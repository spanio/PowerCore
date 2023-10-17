#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "PWM_Drv_Config.h" // Defines the PWM Channel enumeration
// Platform Includes
#include "MessageRouter.h"
// Other Includes
#include <stdbool.h>
#include <stdint.h>

// Common configuration structure passed to the module initialization function
// Data is generally defined in the board-specific configuration file
typedef struct
{
    // The number of items in the gpioConfigData - calculated by compiler
    // Note that this must match PWM_DRV_CHANNEL_COUNT
    uint16_t numConfigItems;
    // PWM channel configuration data
    const struct PWM_Drv_ChannelConfig_s *channelConfigArray;

    // The number of items in the pwmGroupConfigData - calculated by compiler
    // Note that this must match PWM_DRV_GROUP_COUNT
    uint16_t numGroupConfigItems;
    // Group configuration data
    const struct PWM_Drv_GroupConfig_s *groupConfigArray;
} PWM_Drv_Config_t;


/*******************************************************************************
// Public Function Declarations
*******************************************************************************/
void PWM_Drv_Config(void);

void PWM_Drv_Resume(void);

void PWM_Drv_Suspend(void);

/** Description:
 *    This function initializes the PWM module and configures the
 *    timer module used to generate the PWM output. After this
 *    function is called, it is expected that the module is ready
 *    to operate.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
bool PWM_Drv_Init(const uint32_t moduleID, const PWM_Drv_Config_t *configData);

/** Description:
 *    This function gets the current frequency in Hertz for the given PWM
 *    channel.
 * Returns:
 *    uint32_t :  An unsinged 32-bit value that
 *                represents the current PWM frequency in Hertz.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
uint32_t PWM_Drv_GetFrequencyHz(PWM_Drv_Channel_t pwmChannel);

/** Description:
 *    This function gets the current frequency in Hertz for the given PWM
 *    channel.
 * Parameters:
 *    pwmChannel    :  The enumerated channel value for which this function will set the frequency.
 *    frequencyHz   :  An unsigned 32-bit value that
 *                     specifies the new PWM frequency in Hertz.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void PWM_Drv_SetFrequencyHz(PWM_Drv_Channel_t pwmChannel, uint32_t frequencyHz);

/** Description:
 *    This function gets the current frequency in Hertz for the given PWM
 *    channel.
 * Parameters:
 *    pwmChannel    :  The enumerated channel value for which this function will set the parameters.
 *    frequencyHz   :  An unsigned 32-bit value that specifies the new PWM frequency in Hertz.
 *    percentFP216  :  The 32-bit fixed point representation of the duty cycle percent specified in fixed point 2^16 format.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void PWM_Drv_SetFrequencyDutyCycle(PWM_Drv_Channel_t pwmChannel, uint32_t frequencyHz, uint32_t percentFP216);

/** Description:
 *    This function gets the current duty cycle percentage for the specified PWM channel enumeration.
 * Parameters:
 *    pwmChannel :  The enumerated channel value for which this function will return the current duty cycle.
 * Returns:
 *    uint32_t: The fixed point representation of the duty cycle percent specified in fixed point 2^16 format.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
uint32_t PWM_Drv_GetDutyCycle(const PWM_Drv_Channel_t pwmChannel);

/** Description:
 *    This function sets the duty cycle as a percentage for the given channel.
 * Parameters:
 *    pwmChannel    :  The enumerated channel value for which this function will set the frequency.
 *    percentFP216  :  The 32-bit fixed point representation of the duty cycle percent specified in fixed point 2^18 format (16.16).
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void PWM_Drv_SetDutyCycle(PWM_Drv_Channel_t pwmChannel, uint32_t percentFP216);

/** Description:
 *    This function gets the current dead time for the specified PWM channel enumeration.
 * Parameters:
 *    pwmChannel :  The enumerated channel value for which this function will return the current duty cycle.
 * Returns:
 *    uint16_213_t: The fixed point representation of the duty cycle percent specified in fixed point 2^13 format.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
uint16_t PWM_Drv_GetDeadtime(const PWM_Drv_Channel_t pwmChannelVal);

/** Description:
 *    This function sets the duty cycle as a percentage for the given channel.
 * Parameters:
 *    pwmChannel    :  The enumerated channel value for which this function will set the frequency.
 *    percentFP213  :  The raw 16-bit dead time value to be set.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void PWM_Drv_SetDeadtime(PWM_Drv_Channel_t pwmChannel, uint16_t deadtime);


/*******************************************************************************
// Command Processor Functions
*******************************************************************************/

/** Description:
 *    This function is called by the Message Router module to
 *    process the Get Frequency (Hz) command.
 * Parameters:
 *    message :  The common message object that specifies both the
 *               command and response for the given message. It is
 *               expected that the response will be set in this
 *               object when the function is complete.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void PWM_Drv_MessageRouter_GetFrequencyHz(MessageRouter_Message_t *const message);

/** Description:
 *    This function is called by the Message Router module to
 *    process the Set Frequency (Hz) command.
 * Parameters:
 *    message :  The common message object that specifies both the
 *               command and response for the given message. Any
 *               \parameters for the command are set in this object.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void PWM_Drv_MessageRouter_SetFrequencyHz(MessageRouter_Message_t *const message);

/** Description:
 *    This function is called by the Message Router module to
 *    process the Get Duty Cycle command.
 * Parameters:
 *    message :  The common message object that specifies both the
 *               command and response for the given message. It is
 *               expected that the response will be set in this
 *               object when the function is complete.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void PWM_Drv_MessageRouter_GetDutyCycle(MessageRouter_Message_t *const message);

/** Description:
 *    This function is called by the Message Router module to
 *    process the Set Duty Cycle command.
 * Parameters:
 *    message :  The common message object that specifies both the
 *               command and response for the given message. Any
 *               \parameters for the command are set in this object.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void PWM_Drv_MessageRouter_SetDutyCycle(MessageRouter_Message_t *const message);

void PWM_Drv_MessageRouter_GetDeadtime(MessageRouter_Message_t *const message);

void PWM_Drv_MessageRouter_SetDeadtime(MessageRouter_Message_t *const message);

void PWM_Drv_MessageRouter_GetEnableState(MessageRouter_Message_t *const message);

void PWM_Drv_MessageRouter_SetEnableState(MessageRouter_Message_t *const message);

bool PWM_Drv_GetEnableState(PWM_Drv_Channel_t pwmChannel);

void PWM_Drv_SetEnableState(PWM_Drv_Channel_t pwmChannel, bool enableState);

void PWM_Drv_SetDutyCycle_Test(PWM_Drv_Channel_t pwmChannel, float fDutyCycle01);

void PWM_Drv_MessageRouter_SetDutyCycle_Test(MessageRouter_Message_t *message);

#ifdef __cplusplus
extern "C"
}
#endif
