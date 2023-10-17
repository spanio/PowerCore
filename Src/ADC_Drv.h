#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

// Module Includes
#include "ADC_Drv_Config.h"
// Platform Includes
#include "MessageRouter.h"
// Other Includes
#include <stdint.h>

typedef struct
{

    uint32_t numConfigItems;
    const struct ADC_Drv_Data_s *dataPtr;
} ADC_Drv_Config_t;

/** Description:
 *    This function initializes (but does not start) the entire
 *    fast ADC module. After this function is called, it is
 *    expected the module is ready to be started.
 *
 */
bool ADC_Drv_Init(const uint32_t moduleID, const ADC_Drv_Config_t *configData);

void ADC_Drv_Update(void);

/** Description:
 *    This function starts or stops continuous conversion of all
 *    configured fast ADC channels in the system.
 * Parameters:
 *    enableState :  The new state for the fast ADC module. If
 *                   disabled, all readings will be equal to
 *                   INVALID_ADC_VALUE.<p /><p />Values\: true \-
 *                   Enable the ADC module, false \- Disables the
 *                   ADC module.
 *
 */
void ADC_Drv_SetEnableState(bool enableState);

void ADC_Drv_StoreResults(void);

/** Description:
 *    This function returns the most recent reading for the given
 *    channel.
 * Parameters:
 *    channel :  The enumerated value that identifies the channel
 *               for which the value is to be returned.
 * Returns:
 *    uint16_t : The value representing the converted ADC value.
 *    Not that this value is only valid if the value has been read
 *    for the channel. The value is specified in ADC counts. For a
 *    12-bit ADC this would be a value of the range 0-4095.
 *
 */
uint16_t ADC_Drv_GetValue(ADC_Drv_Channel_t const channel);

/** Description:
 *    This function is called by the Message Router module to
 *    process the ADC_Drv_GetValue message for the fast ADC
 *    module.
 * Parameters:
 *    message :  The internal Message Router object that contains
 *               both the command parameters and the buffer where
 *               the response is to be placed for this message.
 *
 */
void ADC_Drv_MessageRouter_GetADCValue(MessageRouter_Message_t *const message);

#ifdef __cplusplus
extern "C"
}
#endif
