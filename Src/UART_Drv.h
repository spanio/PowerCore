/*******************************************************************************
// UART Driver 
// Defines the common, logical interface for UART channels. 
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once


/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "UART_Drv_Config.h" // Defines channel identifiers
// Platform Includes
// Other Includes
#include <stdbool.h> // DEfines C99 boolean type
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


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

typedef struct
{
    uint32_t numConfigItems;
    const struct UART_Drv_Data_s *dataPtr;
} UART_Drv_Config_t;




/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/*******************************************************************************
// Description:
//    Specifies the configuration tables used for the module and also specifies
//    the unique module Identifier used for routing and error reporting.
// Parameters:
//    moduleId - The numeric module identifier to be used for the module.
//       Note that this value should be unique to to each module in the system.
//    configData - Defines the required configuration data for the module.
//       Note that the data is typically defined in the provided as part of the
//       board-specific configuration files.
// Returns: 
//    bool - The result of the initialization
// Return Value List: 
//    true - The module was initialized successfully
//    false - The module was unable to complete successful initialization.
//    Calls to API functions in the module will have unxepcted results.
*******************************************************************************/
bool UART_Drv_Init(const uint32_t moduleId, const UART_Drv_Config_t *configPtr);

/*******************************************************************************
 // Description:
 //    Returns the actual baud rate for the given channel.  This function can be
 //    used to determine the error for the given clock configuration.
 //    An invalid channel will return 0.
 // Parameters:
 //    channel - The logical identifier of the channel to be read
 // Returns:
 //    uint32_t - The actual baud rate as a decimal value
 // Return Value List:
 //    0: The given channel is invalid or not yet configured
 //    1+: The actual baud rate in use based on the configured peripheral clock/divider
 *******************************************************************************/
uint32_t UART_Drv_GetActualBaudRate(UART_Drv_Channel_t channel);

/*******************************************************************************
// Description:
//    Returns the number of characters that are available to be read from
//    the UART peripheral.
// Parameters:
//    channelId - The logical identifier of the channel to be read
// Returns: 
//    uint16_t - The number of bytes read from the specified channel
// Return Value List: 
//    0: No data is available to be read.
//    1+: One or more characters are available to be read.
*******************************************************************************/
uint16_t UART_Drv_GetNumCharsRX(const UART_Drv_Channel_t channelId);

/*******************************************************************************
// Description:
//    Returns the number of characters that can be succesfully written to the
//    UART peripheral for outgoing transmission.
// Parameters:
//    channelId - The logical identifier of the channel to be written
// Returns: 
//    uint16_t - The number of bytes written to the specified channel
// Return Value List: 
//    0: The UART peripheral is busy and has no room available.
//    1+: The maximum number of characters that can be written to perhiperal
//    for transmission.
*******************************************************************************/
uint16_t UART_Drv_GetNumCharsTX(const UART_Drv_Channel_t channelId);

/*******************************************************************************
// Description:
//    Reads a single character from the read buffer for a UART channel
//    using a non-blocking read. The number of available characters should be
//    checked before reading using UART_Drv_GetNumCharsRX().
// Parameters:
//    channelId - The logical identifier of the channel to be read
// Returns: 
//    uint16_t - The actual character read from the RX buffer. Returns 0 if
//    the channel is invalid or no data is available.
*******************************************************************************/
uint16_t UART_Drv_ReadChar(const UART_Drv_Channel_t channelId);

/*******************************************************************************
// Description:
//    Reads a multiple character from the RX buffer of a UART peripheral
//    into a buffer and returns the number of characters read up to the
//    given length. If no data is available, the function will not block
//    and returns 0 characters read. The buffer will not be modified if
//    no data is available.
// Parameters:
//    channelId - The logical identifier of the channel to be read
// Returns: 
//    uint16_t - The number of bytes read from the specified channel
// Return Value List: 
//    0: No data was read from the RX buffer.
//    1+: One byte was read and placed in the given buffer.
*******************************************************************************/
uint16_t UART_Drv_ReadCharArray(const UART_Drv_Channel_t channelId, uint16_t *const data, const uint16_t maxLength);

/*******************************************************************************
// Description:
//    Write a single character to a UART peripheral for transmit.  The number of
//    characters written is returned. If the peripheral is busy and cannot accept
//    data for transmission, the function will not block and returns 0 characters
//    written.  Note that placing data for transmission does not guarantee it
//    that transmission is complete.  
// Parameters:
//    channelId - The logical identifier of the channel to be written
// Returns: 
//    uint16_t - The number of bytes written to the specified channel (Max 1)
// Return Value List: 
//    0: No data was written to the UART peripheral register.
//    1: One byte was placed in to the UART peripheral register.
*******************************************************************************/
uint16_t UART_Drv_WriteChar(const UART_Drv_Channel_t channelId, const uint16_t data);

/*******************************************************************************
// Description:
//    Write multiple characters to a UART peripheral for transmit.  The number of
//    characters written is returned. If the peripheral cannot accept ALL DATA
//    for transmission, the function will not block and writes up to the available
//    limit.  Note that the caller should not expect that all data will be written.
// Parameters:
//    channelId - The logical identifier of the channel to be written
// Returns: 
//    uint16_t - The number of bytes written to the specified channel
// Return Value List: 
//    0: No data was written to the UART peripheral register.
//    1+: One or more bytes was placed in to the UART peripheral register.
*******************************************************************************/
uint16_t UART_Drv_WriteCharArray(const UART_Drv_Channel_t channelId, uint16_t *const data, const uint16_t dataLength);

void UART_Drv_Write(const UART_Drv_Channel_t channel, uint16_t *const data, const uint16_t length);
void UART_Drv_Update(void);

/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif


