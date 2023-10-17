/*******************************************************************************
// UART Driver Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
//#include "Interrupt_Drv.h" // ISR Callbacks
// Other Includes
#include "interrupt.h" // DriverLib Interrupt API


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


typedef enum
{
    // The channel used for debug communication
    UART_DRV_CHANNEL_DEBUG,
   // Defines the number of enumerated UART channels configured for the system
   UART_DRV_CHANNEL_COUNT
} UART_Drv_Channel_t;


/*******************************************************************************
// Public Variable Declarations
*******************************************************************************/


// End of C Binding Section
#ifdef __cplusplus
}
#endif

