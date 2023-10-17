/*******************************************************************************
// ADC Driver Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "ADC_Drv_Config.h"
// Platform Includes
#include "Interrupt_Drv.h"
// Other Includes
#include "adc.h" //TI DriverLib
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


typedef struct ADC_Drv_Data_s {
    // Logical channel identifier
    // Note that this is expected to match the ADC_Drv_Channel_t enumeration such that
    // the first item configuration corresponds to the the first enumerated value
    ADC_Drv_Channel_t channelId;

    // ADC conversion unit base
    // ADCx_BASE
    uint32_t base;

    // ADC conversion result address for the channel
    // ADCxRESULT_BASE (x: A..D)
    uint32_t resultBase;

    // SOC for the channel
    // ADC_SOC_NUMBERx (x: 0..15)
    ADC_SOCNumber socNumber;

    // Trigger source, ex: ADC_TRIGGER_EPWM2_SOCA
    ADC_Trigger triggerSource;

    // ADC channel, ex: ADC_CH_ADCIN0
    ADC_Channel channel;

    // Sample window time in nanoseconds
    uint32_t sampleWindowNS;
} ADC_Drv_Data_t;



// End of C Binding Section
#ifdef __cplusplus
}
#endif

