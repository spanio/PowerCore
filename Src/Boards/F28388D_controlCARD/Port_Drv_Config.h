/*******************************************************************************
// Port Driver
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once


/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
//#include "Port_Drv.h"
// Platform Includes
// Other Includes
#include "gpio.h" // TI DriverLib
#include <stdbool.h>  // Defines C99 boolean type
#include <stdint.h>  // Defines C99 integer types


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

// Pin level value for low (0)
#define  PORT_PIN_LEVEL_LOW        (0)
// Pin level value for high (1)
#define  PORT_PIN_LEVEL_HIGH       (1)


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

// We map ports directly to the hardware.
// Logical definitions used for Ports GPIO_PORT_A..GPIO_PORT_F
typedef enum
{
    // GPIO 0..31 (32)
    PORT_DRV_PORT_ID_A,
    // GPIO 32..63 (32)
    PORT_DRV_PORT_ID_B,
    // GPIO 64..95 (32)
    PORT_DRV_PORT_ID_C,
    // GPIO 96..127 (32)
    PORT_DRV_PORT_ID_D,
    // GPIO 128..159 (32)
    PORT_DRV_PORT_ID_E,
    // GPIO 160..168 (9)
    PORT_DRV_PORT_ID_F,
    // Defines the total of valid port identifiers
    PORT_DRV_PORT_ID_COUNT
} Port_Drv_PortId_t;

// We map ports directly to the hardware.
// Logical definitions used for Ports GPIO_PORT_A..GPIO_PORT_F
typedef GPIO_Port Port_Drv_Port_t;


// Processor pin number (0-168U)
// Can verify with GPIO_isPinValid() in TI DriverLib
typedef uint32_t GPIO_Pin_t;

// Port Pin number (0-31 max)
// TI DriverLib typically uses a single GPIO pin number rather than a Port/Pin combination
// For example, GPIO34 is specified by passing 34 as the pin rather than PortB/Pin3
typedef uint16_t Port_Drv_Port_Pin_t;

// Defines port/pin combination used by TI DriverLib
typedef struct
{
    // GPIO_PORT_A..GPIO_PORT_F
   GPIO_Port portId;
   // Pin index of the port (0..31)
   Port_Drv_Port_Pin_t pinIndex;
} PortPin_t;

typedef uint32_t Pin_Mux_Config_t;

// Structure to define the configuration options for a GPIO input
typedef struct
{
    // Standard pin mode defines floating input or pullup input configuration
    // Value is passed to GPIO_setPadConfig()
    // GPIO_PIN_TYPE_STD = Floating input
    // GPIO_PIN_TYPE_PULLUP = Pull-up enable for input
    uint32_t inputPinType;

    // Qualification mode passed to GPIO_setQualificationMode()
    // Default =  GPIO_QUAL_SYNC (Synchronization to SYSCLK)
    // GPIO_QUAL_ASYNC = No synchronization when peripheral controls sync
    GPIO_QualificationMode qualificationMode;

   // Defines if pin input inversion should be enabled (TRUE = Active Low)
   // Value is OR-ed using GPIO_PIN_TYPE_INVERT in GPIO_setPadConfig()
   bool isInverted;
} Port_Drv_InputConfig_t;


// Structure to define the configuration options for a GPIO output
typedef struct
{
    // Standard pin mode defines push-pull or open drain output configuration
    // Value is passed to GPIO_setPadConfig()
    // GPIO_PIN_TYPE_STD = Push-pull output    
    // GPIO_PIN_TYPE_OD = Open-drain on output
    uint32_t outputPinType;

    // Defines the initial state for a GPIO pin during initialization
    // GPIO_LEVEL_LOW or GPIO_LEVEL_HIGH
    uint32_t initialLevel;
} Port_Drv_OutputConfig_t;



/*******************************************************************************
// Public Function Declarations
*******************************************************************************/


/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif



