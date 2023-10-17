/*******************************************************************************
// UART Driver Configuration Interface
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "UART_Drv_Config.h"
// Platform Includes
// Other Includes
#include "SysCtl.h" // TI DriverLib Peripheral Clock
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

typedef enum
{
    UART_DRV_PARITY_NONE,
    UART_DRV_PARITY_EVEN,
    UART_DRV_PARITY_ODD,
    UART_DRV_PARITY_COUNT
} UART_Drv_Parity_t;

typedef struct {
    // PIE Interrupt source Interrupt_register() function - Use mask defined in hw_ints.h
    // Ex. INT_SCIA_RX defines SCIA Receive Interrupt (Source 1 of Group 9)
    // TODO - configure based on uartBase
    uint32_t interruptNumber;
    // TODO - ISR function passed to Interrupt_register() function
    void (*callback)(void);
} UART_Drv_InterruptConfig_t;

typedef struct UART_Drv_Data_s {
    // Logical channel identifier
    // Note that this is expected to match the UART_Drv_Channel_t enumeration such that
    // the first item configuration corresponds to the the first enumerated value
    UART_Drv_Channel_t channelId;
      // Baud rate is calculated using the low speed clock (from SysCtl_getLowSpeedClock()) and passed to SCI_setConfig()
      uint32_t baudRate;
      // Decimal value representing the character length (1-8)
      // This value will be converted internally to a bit mask (such as SCI_CONFIG_WLEN_8) passed to SCI_setConfig()
      // Note that logical, decimal values do not match the bit mask (SCI_CONFIG_WLEN_8 is decimal value 7)
      uint32_t bitLength;
      // Decimal value representing the number of stop bits transmitter (1 or 2)
      // Note that the receiver will only check for 1 stop bit.
      // This value will be converted internally to a bit mask (such as SCI_CONFIG_STOP_ONE) passed to SCI_setConfig()
      // Note that logical, decimal values do not match the bit mask (SCI_CONFIG_STOP_ONE is decimal value 0)
      uint32_t stopBits;
      // Parity bit config will be converted to SCI_ParityType passed to SCI_setConfig()
      // This value will be converted internally to a bit mask (such as SCI_CONFIG_WLEN_8) passed to SCI_setConfig()
      // Note that SCI_ParityType enumeration is not sequential and actually represents a bit mask
      UART_Drv_Parity_t parity;
      // UART Peripheral Address from hw_memmap.h (Ex. SCIA_BASE)
      uint32_t uartBase;
      // Peripheral Clock enumeration defined by SysCtl_PeripheralPCLOCKCR type and passed to SysCtl_enablePeripheral()
      // since peripheral clocks must be enabled for each peripheral.
      // Note that SysCtl_PeripheralPCLOCKCR enumeration is not sequential and actually represents a bit mask.
      // TODO - configure based on uartBase
      SysCtl_PeripheralPCLOCKCR peripheral;
      // Interrupt Handler for TX
      // TODO - configure based on uartBase
      UART_Drv_InterruptConfig_t txIRQConfig;
      // Interrupt Handler for RX
      // TODO - configure based on uartBase
      UART_Drv_InterruptConfig_t rxIRQConfig;
} UART_Drv_Data_t;


// End of C Binding Section
#ifdef __cplusplus
}
#endif

