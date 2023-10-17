/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "ADC_Drv.h"
#include "ADC_Drv_Config.h" // Defines configuration structure
#include "ADC_Drv_ConfigTypes.h" // Defines configuration structure
// Platform Includes
#include "MessageRouter.h"
// Other Includes

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/
const ADC_Drv_Data_t adcData[ADC_DRV_CHANNEL_COUNT] =
{
   // TODO - Remove - A0 and A1 not connected on TestBed - connected to DAC
   // ADC A
   {.channelId = ADC_DRV_CHANNEL_ID_A0, .base = ADCA_BASE, .resultBase =  ADCARESULT_BASE, .socNumber = ADC_SOC_NUMBER0, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN0, .sampleWindowNS = 75U},
   {.channelId = ADC_DRV_CHANNEL_ID_A1, .base = ADCA_BASE, .resultBase =  ADCARESULT_BASE, .socNumber = ADC_SOC_NUMBER1, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN1, .sampleWindowNS = 75U},
   {.channelId = ADC_DRV_CHANNEL_ID_AC_SNS1, .base = ADCA_BASE, .resultBase =  ADCARESULT_BASE, .socNumber = ADC_SOC_NUMBER2, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN2, .sampleWindowNS = 75U},
   {.channelId = ADC_DRV_CHANNEL_ID_AC_SNS2, .base = ADCA_BASE, .resultBase =  ADCARESULT_BASE, .socNumber = ADC_SOC_NUMBER3, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN3, .sampleWindowNS = 75U},
   {.channelId = ADC_DRV_CHANNEL_ID_AC_SNS3, .base = ADCA_BASE, .resultBase =  ADCARESULT_BASE, .socNumber = ADC_SOC_NUMBER4, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN4, .sampleWindowNS = 75U},
   {.channelId = ADC_DRV_CHANNEL_ID_AC_SNS4, .base = ADCA_BASE, .resultBase =  ADCARESULT_BASE, .socNumber = ADC_SOC_NUMBER5, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN5, .sampleWindowNS = 75U},

    // ADC B                                     
    {.channelId = ADC_DRV_CHANNEL_ID_I_PRI1_SNS, .base = ADCB_BASE, .resultBase = ADCBRESULT_BASE, .socNumber = ADC_SOC_NUMBER0, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN0, .sampleWindowNS = 75U},
    {.channelId = ADC_DRV_CHANNEL_ID_I_MID2_SNS, .base = ADCB_BASE, .resultBase = ADCBRESULT_BASE, .socNumber = ADC_SOC_NUMBER1, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN1, .sampleWindowNS = 75U},
    {.channelId = ADC_DRV_CHANNEL_ID_I_MID3_SNS, .base = ADCB_BASE, .resultBase = ADCBRESULT_BASE, .socNumber = ADC_SOC_NUMBER2, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN2, .sampleWindowNS = 75U},
    {.channelId = ADC_DRV_CHANNEL_ID_I_SEC1_SNS, .base = ADCB_BASE, .resultBase = ADCBRESULT_BASE, .socNumber = ADC_SOC_NUMBER3, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN3, .sampleWindowNS = 75U},
    {.channelId = ADC_DRV_CHANNEL_ID_VSEC_SNS,   .base = ADCB_BASE, .resultBase = ADCBRESULT_BASE, .socNumber = ADC_SOC_NUMBER4, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN4, .sampleWindowNS = 75U},
    {.channelId = ADC_DRV_CHANNEL_ID_I_MID4_SNS, .base = ADCB_BASE, .resultBase = ADCBRESULT_BASE, .socNumber = ADC_SOC_NUMBER5, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN5, .sampleWindowNS = 75U},

    // ADC C not currently used

    // ADC D
    {.channelId = ADC_DRV_CHANNEL_ID_VPRI_SNS,   .base = ADCD_BASE, .resultBase = ADCDRESULT_BASE, .socNumber = ADC_SOC_NUMBER0, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN0, .sampleWindowNS = 75U},
    {.channelId = ADC_DRV_CHANNEL_ID_I_MID1_SNS, .base = ADCD_BASE, .resultBase = ADCDRESULT_BASE, .socNumber = ADC_SOC_NUMBER1, .triggerSource = ADC_TRIGGER_EPWM2_SOCA, .channel = ADC_CH_ADCIN1, .sampleWindowNS = 75U}
};

const ADC_Drv_Config_t adcConfig =
{

    .numConfigItems = sizeof(adcData)/sizeof(ADC_Drv_Data_t),
    .dataPtr = adcData
};

// This table provides a list of commands for this module. The primary purpose
// is to link each Command ID to its corresponding message handler function
const MessageRouter_CommandTableItem_t adcMessageTable[] =
{
   // {Command ID, Message Handler Function Pointer}
   { 0x01, ADC_Drv_MessageRouter_GetADCValue },
};


const MessageRouter_Data_t adcMessageConfig =
{
 //.moduleID = SYSTEM_MODULE_ID,
 .numCommands = sizeof(adcMessageTable)/sizeof(MessageRouter_CommandTableItem_t),
 .commandTable = adcMessageTable
};


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/
