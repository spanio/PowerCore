/*******************************************************************************
// Core Serial Protocol Module
*******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
// Platform Includes
#include "MessageRouter.h"
#include "UART_Drv.h"
// Other Includes
#include <stdint.h>

/*******************************************************************************
// Public Constant Definitions
*******************************************************************************/

/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

// Encoding type enumeration
typedef enum
{
   // Protocol uses binary encoding for protocol data
   SERIAL_ENCODING_BINARY,
   // Protocol uses ASCII-coded hex encoding for protocol data
   SERIAL_ENCODING_ASCII_CODED_HEX
} Serial_Encoding_t;

/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/** Description:
 *    This function initializes the serial module and calls the
 *    UART driver initialization function for each configure port.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
bool Serial_Init(const uint32_t moduleId, const void *configData);


/** Description:
 *    This is the scheduled update function that will check the receive buffer
 *    for a complete command.  If found, the corresponding command will be
 *    executed via a call to the Message Router module.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void Serial_Update(void);

/** Description:
 *    Reset the TX/RX statistics for the given port.
 * Parameters:
 *    channel - The configured UART port that is to be reset
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void Serial_ResetStats(const UART_Drv_Channel_t channel);

/** Description:
 *    This function is called to transmit data to the given UART. The data that
 *    is passed into this function will be placed in the circular transmit
 *    buffer.  If we are not currently transmitting, the TX ISR must be started
 *    by writing the first byte. After the first byte, each byte from the
 *    circular buffer is sent via the transmit interrupt until the buffer
 *    is empty.
 *    NOTE:  Never send data with more than TX_BUFFER_SIZE values, or the data will
 *           wrap around the circular TX buffer.
 * Parameters:
 *    channel - The configured UART pin that is to be written
 *    data - A pointer to the data that is to be sent.
 *    dataLength - The number of bytes in the data buffer that is to be sent.
 *    encoding - Specifies the encoding to be used when placing data in the
 *    transmit buffer.  If ASCII, each byte will be converted (Ex. ASCII
 *    encoded Hex, 0x0F = "0F")
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void Serial_Send(const UART_Drv_Channel_t channel, uint16_t *const data, const uint16_t dataLength,
                       const Serial_Encoding_t encoding);

/** Description:
 *    This function converts one or two ASCII-coded hex cahracters to a 16-bit
 *    value.
 * Parameters:
 *    hexCharacters    : pointer to the buffer of ASCII character to be converted.
 *    numHexCharacters : The number of characters in the given buffer.
 * Returns:
 *    uint16_t: The converted numeric value of the given ASCII-coded hex value.
 * History:
 *    * Date: Function created (EJH)
 *
 */
uint16_t Serial_ConvertAsciiHexStringToNumeric(uint16_t *const hexCharacters, const uint16_t numHexCharacters);


/** Description:
 *    This is the command handler used for querying serial TX/RX statistics for
 *    a given port.
 * Parameters:
 *    message :  A pointer to a common Message Router message
 *               object. The response is expected to be placed in
 *               this object.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void Serial_MessageRouter_GetSerialStatistics(MessageRouter_Message_t *const message);

/** Description:
 *    This is the command handler used for resetting serial TX/RX statistics
 *    for a given port.
 *    Parameters:
 *       message :  A pointer to a common Message Router message object. The
 *       response is expected to be placed in this object.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
void Serial_MessageRouter_ResetSerialStatistics(MessageRouter_Message_t *const message);

#ifdef __cplusplus
extern "C"
}
#endif
