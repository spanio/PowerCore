/*******************************************************************************
// Port Driver Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Port_Drv_Config.h"
// Platform Includes
// Other Includes
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

// This is the structure for each entry in the GPIO configuration table
typedef struct Port_Drv_Data_s
{
    // Defines the input or output state for the pin - passed to GPIO_setDirectionMode
    // GPIO_DIR_MODE_IN or GPIO_DIR_MODE_OUT
    GPIO_Direction direction;

   // The provides a structure to define the device-specific port and pin
    PortPin_t devicePin;

    // Pin Mux for GPIO or Alt. Function
    // Passed to GPIO_SetPinConfig() and defined in pin_map.h
    // Ex. GPIO_31_GPIO31 or GPIO_28_SCIRXDA
    Pin_Mux_Config_t pinMuxConfiguration;

   // Configuration settings specific to either GPIO input or output configuration
   union {
      // Options when pin is configured as input
      Port_Drv_InputConfig_t gpioInput;
      // Option when pin is configured as output
      Port_Drv_OutputConfig_t gpioOutput;
   } gpioConfig;
} Port_Drv_Data_t;


// End of C Binding Section
#ifdef __cplusplus
}
#endif

