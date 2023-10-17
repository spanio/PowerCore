/*******************************************************************************
// ADC Driver Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
// Other Includes
#include <stdint.h> // integer limits


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

// Max ADC value for 12-bit ADC channels
#define ADC_DRV_MAX_VALUE (0x0FFFU)

// Value used to denote that an ADC reading is invalid (such as startup).
// Typically this is set such that the reading would cause a converted value to be in the error range.
// Ex. 0xFFFF would convert to a high current exceeding maximum amperage and would prevent starting
// until a valid below the hysteresis value has been read
#define ADC_DRV_INVALID_VALUE (UINT16_MAX)

/*******************************************************************************
// Public Type Declarations
*******************************************************************************/


typedef enum
{
   // TODO - Remove - A0 and A1 not connected on TestBed - connected to DAC
   ADC_DRV_CHANNEL_ID_A0, // 0: ADCINA0
   ADC_DRV_CHANNEL_ID_A1, // 1: ADCINA1
   ADC_DRV_CHANNEL_ID_AC_SNS1, // 2: ADCINA2
   ADC_DRV_CHANNEL_ID_AC_SNS2, // 3: ADCINA3
   ADC_DRV_CHANNEL_ID_AC_SNS3, // 4: ADCINA4
   ADC_DRV_CHANNEL_ID_AC_SNS4, // 5: ADCINA5
   ADC_DRV_CHANNEL_ID_I_PRI1_SNS, // 6: ADCINB0
   ADC_DRV_CHANNEL_ID_I_MID2_SNS, // 7: ADCINB1
   ADC_DRV_CHANNEL_ID_I_MID3_SNS, // 8: ADCINB2
   ADC_DRV_CHANNEL_ID_I_SEC1_SNS, // 9: ADCINB3
   ADC_DRV_CHANNEL_ID_VSEC_SNS, // 10: ADCINB4
   ADC_DRV_CHANNEL_ID_I_MID4_SNS, // 11: ADCINB5
   ADC_DRV_CHANNEL_ID_VPRI_SNS, // 12: ADCIND0
   ADC_DRV_CHANNEL_ID_I_MID1_SNS, // 13: ADCIND1

   // Identifies the number of enumerated values -- not a valid ADC channel
   // This must be the last item and the first item must start at 0
   ADC_DRV_CHANNEL_COUNT
} ADC_Drv_Channel_t;



// End of C Binding Section
#ifdef __cplusplus
}
#endif

