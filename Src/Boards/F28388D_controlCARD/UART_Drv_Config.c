/*******************************************************************************
// UART Driver Configuration Data
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "UART_Drv.h"
#include "UART_Drv_Config.h" // Defines configuration structure
#include "UART_Drv_ConfigTypes.h" // Defines configuration structure
// Platform Includes
// Other Includes
#include "sysctl.h" // Peripheral clocks

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

// Enumeration of the UART channels configured in the system


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/
const UART_Drv_Data_t uartData[UART_DRV_CHANNEL_COUNT] =
{
 // --- SCIA - 28/29 - Debug/FTDI - 96000
 //#define DEVICE_GPIO_PIN_SCIRXDA     28U             // GPIO number for SCI RX
 //#define DEVICE_GPIO_PIN_SCITXDA     29U             // GPIO number for SCI TX
 //#define DEVICE_GPIO_CFG_SCIRXDA     GPIO_28_SCIRXDA // "pinConfig" for SCI RX
 //#define DEVICE_GPIO_CFG_SCITXDA     GPIO_29_SCITXDA // "pinConfig" for SCI TX
     {
        // First channel is debug
        .channelId = UART_DRV_CHANNEL_DEBUG,
        // 9600bps for polled implementation
        .baudRate = 9600UL,
        // 8 bit characters
        .bitLength = 8U,
        // Use 1 stop bit
        .stopBits = 1U,
        // No parity bit
        .parity = UART_DRV_PARITY_NONE,
        // UART A peripheral address
        .uartBase = SCIA_BASE,
        // Peripheral clock for this SCI peripheral
        // TODO - configure based on uartBase
        .peripheral = SYSCTL_PERIPH_CLK_SCIA,
         // IRQ not used currently
     },
};

// Configuration data passed at initialization
extern const UART_Drv_Config_t uartConfig =
{
    .numConfigItems = sizeof(uartData)/sizeof(UART_Drv_Data_t),
    .dataPtr = uartData
};




/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

