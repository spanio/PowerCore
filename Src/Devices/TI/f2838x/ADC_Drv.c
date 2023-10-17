/*******************************************************************************
// ADC Driver - TI F2838xD Implementation
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "ADC_Drv.h"
#include "ADC_Drv_Config.h"
// Platform Includes
#include "Port_Drv.h"
#include "Sys.h" // Delay
// Other Includes
#include "adc.h" // TI DriverLib ADC API
#include "epwm.h" // TI DriverLib PWM API for Sync
#include <stdbool.h> // Defines C99 boolean type
#include <stdint.h> // Defines C99 integer types

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

// Max ADC clock is 50MHz - Must prescale /4 for 200MHz system clock
#define DEFAULT_PRESCALE_VALUE ADC_CLK_DIV_4_0

// 1000 ns when clock is 1MHz
#define NS_PER_MHZ (1000U)

// Driver currently only implements 12-bit resolution mode
#define ADC_RESOLUTION_MODE (ADC_RESOLUTION_12BIT)

// From datasheet, 12-bit conversion is 75ns
#define ADC_12_BIT_CONVERSION_NS (75)

// From datasheet, 16-bit conversion is 320ns
#define ADC_16_BIT_CONVERSION_NS (320)

// Delay for ADC units to power up
#define REQUIRED_POWER_UP_DELAY_US (500U)


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

typedef enum
{
    ADC_DRV_HW_UNIT_A,
    ADC_DRV_HW_UNIT_B,
    ADC_DRV_HW_UNIT_C,
    ADC_DRV_HW_UNIT_D,
    // The total number of hardware units supported by the driver
    ADC_DRV_HW_UINT_COUNT,
} ADC_Drv_HW_Unit_t;

typedef struct
{
    ADC_Drv_HW_Unit_t hwUnitId;
    SysCtl_PeripheralPCLOCKCR adcClk;
    uint32_t adcBase;
} ADC_Drv_HW_Unit_Config_t;

// This structure defines the internal variables used by the module
typedef struct
{
   // Configuration Table passed at Initialization
   ADC_Drv_Config_t *adcConfig;

   uint16_t adcResult[ADC_DRV_CHANNEL_COUNT];

   // Initialization state for the module
   bool isInitialized;

   // Enable state for the module
   bool enableState;
} ADC_Drv_Status_t;


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The variable used for holding all internal data for this module.
static ADC_Drv_Status_t status;

static const ADC_Drv_HW_Unit_Config_t hwUnitConfig[ADC_DRV_HW_UINT_COUNT] =
{
   { ADC_DRV_HW_UNIT_A, SYSCTL_PERIPH_CLK_ADCA, ADCA_BASE },
   { ADC_DRV_HW_UNIT_B, SYSCTL_PERIPH_CLK_ADCB, ADCB_BASE },
   { ADC_DRV_HW_UNIT_C, SYSCTL_PERIPH_CLK_ADCC, ADCC_BASE },
   { ADC_DRV_HW_UNIT_D, SYSCTL_PERIPH_CLK_ADCD, ADCD_BASE }
};

/*******************************************************************************
// Private Function Declarations
*******************************************************************************/


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

// Module initiazation
bool ADC_Drv_Init(const uint32_t moduleID, const ADC_Drv_Config_t *configData)
{
    // Default module to uninitialized and not enabled
    status.isInitialized = false;
    status.enableState = false;

    // First, validate the given parameter is valid
    if (configData)
    {
        if (NULL != configData)
        {
            // Store the given configuration table
            status.adcConfig = (ADC_Drv_Config_t *)configData;

            // TODO: Verify table size
            if (ADC_DRV_CHANNEL_COUNT == status.adcConfig->numConfigItems)
            {
                // Loop through configuration and enable each ADC HW unit
                for (int i= 0; i < ADC_DRV_HW_UINT_COUNT; i++)
                {
                    // Enable ADC clock -- SysCtl_PeripheralPCLOCKCR
                    SysCtl_enablePeripheral(hwUnitConfig[i].adcClk);

                    // Max ADC clock is 50MHz - Must prescale /4 for 200MHz system clock
                    ADC_setPrescaler(hwUnitConfig[i].adcBase ADCA_BASE, ADC_CLK_DIV_4_0);

                    // Enable ADC conversion unit
                    ADC_enableConverter(hwUnitConfig[i].adcBase);
                }

                // Must delay  to allow converter to power up -- 500us for F28x
                // Note: F27x devices require 1000us
                DEVICE_DELAY_US(REQUIRED_POWER_UP_DELAY_US);

                // Get frequency in MHz (Ex. 200MHz) for converting acquisition window
                uint32_t clockMHz = Sys_GetClockFrequencyHz()/1000000UL;

                // Verify configuration data table is value
                if (status.adcConfig->dataPtr)
                {
                    // Loop through and enable each channel
                    for (int i=0; i < status.adcConfig->numConfigItems; i++)
                    {
                        // From datasheet 12-bit resolution is 75ns conversion, 16-bit is 320ns
                        // Convert conversion time to clock cycles
                        // Ex: 75ns = (200MHz * 75ns)/1000ns/MHz = 15;
                        uint32_t acqPeriod = (clockMHz * status.adcConfig->dataPtr[i].sampleWindowNS)/NS_PER_MHZ;
                        ADC_setupSOC(status.adcConfig->dataPtr[i].base,
                                     status.adcConfig->dataPtr[i].socNumber,
                                     status.adcConfig->dataPtr[i].triggerSource,
                                     status.adcConfig->dataPtr[i].channel,
                                     status.adcConfig->dataPtr[i].sampleWindowNS);
                    }

                    // Set to initialized and configured
                    status.isInitialized = true;
                    status.enableState = true;
                }
            }
        }
    }

    // Return initialization state
    return(status.isInitialized);
}

// Get ADC value for the given channel
uint16_t ADC_Drv_GetValue(ADC_Drv_Channel_t const channel)
{
   uint16_t result = ADC_DRV_INVALID_VALUE;

   // Verify parameters - note that
   if ((status.enableState) && (channel < ADC_DRV_CHANNEL_COUNT))
   {
      // Fetch the previous reading from the ADC result register
       result = ADC_readResult(status.adcConfig->dataPtr[channel].resultBase, status.adcConfig->dataPtr[channel].socNumber);
   }

   return(result);
}


// Get ADC value for the given channel
void ADC_Drv_MessageRouter_GetADCValue(MessageRouter_Message_t *const message)
{
   typedef struct
   {
      uint16_t channelIndex;
   } Command_t;

   typedef struct
   {
      uint16_t channelIndex;
      uint16_t adcValue;
   } Cmd_Response_t;

   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), sizeof(Cmd_Response_t)))
   {
      Command_t *command = (Command_t *)message->commandParams.data;
      Cmd_Response_t *response = (Cmd_Response_t *)message->responseParams.data;

      response->channelIndex = command->channelIndex;
      response->adcValue = ADC_Drv_GetValue((ADC_Drv_Channel_t)command->channelIndex);

      MessageRouter_SetResponseSize(message, sizeof(Cmd_Response_t));
   }
}
