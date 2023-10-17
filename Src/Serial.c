/*******************************************************************************
// Core Serial Protocol Module
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "Serial.h"
// Platform Includes
#include "CRCLib.h"
#include "MessageRouter.h"
// Other Includes
#include "UART_Drv.h"        // For UART API
#include "UART_Drv_Config.h" // For UART channel enumeration
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

// Uncomment to constantly send data during every update loop
// This is just used for debugging serial port
//#define DEBUG_SEND_CONSTANT_DATA

// Size of the local circular buffer used for transmitting data (Max 255 in Send function)
#define TX_BUFFER_SIZE (128)

// Size of the local circular buffer used for receiving data.
#define RX_BUFFER_SIZE (128)

// Address used to identifying messages intended for any device
#define BROADCAST_ADDRESS (0xFFU)

// CRC Seed
#define CRC_SEED (0)

//-----------------------------------------------
// Command/Response Constants
//-----------------------------------------------

// This byte indicates the start of an incoming command.
#define COMMAND_START_BYTE ('<')
// Module ID is at the start of the message -- immediately after the start byte
#define MODULE_ID_OFFSET   (0)

// Number of bytes used to denote the length of the data portion
#define DATA_LENGTH_SIZE (1)

// Defines if we are using addressing at the beginning of the message
#define NUM_ADDRESS_BYTES (0)

// This defines the length of a command header in bytes
// Address, if enabled
// Module ID
// Command ID
// Message ID
// Data Length (Part of data buffer, not header)
#define COMMAND_HEADER_SIZE (NUM_ADDRESS_BYTES + sizeof(MessageRouter_MessageItemHeader_t) + DATA_LENGTH_SIZE)

// 2 ASCII characters per byte ("FF")
#define HEX_CHARS_PER_BYTE (2)

#if (16 == CHAR_BIT)
#define HEX_MULTIPLE (2)
#else
#define HEX_MULTIPLE (1)
#endif

/** This defines the length of a command header in ASCII-coded
 * hex.
 */
#define COMMAND_HEADER_SIZE_HASCII (HEX_CHARS_PER_BYTE * COMMAND_HEADER_SIZE)

// This defines the maximum length of command data in bytes.
#define COMMAND_DATA_MAX_SIZE (48)

/** This defines the maximum length of command data in
 * ASCII-coded hex.
 */
#define COMMAND_DATA_MAX_SIZE_HASCII (HEX_CHARS_PER_BYTE * COMMAND_DATA_MAX_SIZE)

// This defines the size of the CRC value used in the message, when enabled
// 2 bytes for CRC-16
#define NUM_CRC_BYTES (2)

// This defines the length of a command footer in bytes
// The footer is only the CRC
#define COMMAND_FOOTER_SIZE        (NUM_CRC_BYTES)
/** This defines the length of a command header in ASCII-coded
 * hex.
 */
#define COMMAND_FOOTER_SIZE_HASCII (HEX_CHARS_PER_BYTE * COMMAND_FOOTER_SIZE)

// This defines the maximum command size in bytes
#define COMMAND_MAX_SIZE        (COMMAND_HEADER_SIZE + COMMAND_DATA_MAX_SIZE + COMMAND_FOOTER_SIZE)
/*This defines the maximum command size in ASCII-coded hex.
 */
#define COMMAND_MAX_SIZE_HASCII (HEX_CHARS_PER_BYTE * COMMAND_MAX_SIZE)

// This is the stop byte used for all incoming commands.
#define COMMAND_STOP_BYTE_1 ('\r')
/*This is an alternate stop byte used for incoming commands.
 */
#define COMMAND_STOP_BYTE_2 ('\n')

//-----------------------------------------------
// Response Packet
//-----------------------------------------------

// This is the start byte used for all outgoing responses.
#define RESPONSE_START_BYTE ('>')

// This defines the length of a response header in bytes -- same as command
// Byte 1: Module ID
// Byte 2: Command ID
// Byte 3: Message ID
// Byte 4: Data Length
#define RESPONSE_HEADER_SIZE        (COMMAND_HEADER_SIZE)
/** This defines the length of a response header in ASCII-coded
 * hex.
 */
#define RESPONSE_HEADER_SIZE_HASCII (HEX_CHARS_PER_BYTE * RESPONSE_HEADER_SIZE)

// This defines the maximum length of response data in bytes
#define RESPONSE_DATA_MAX_SIZE (48)

/** This defines the maximum length of response data in
 * ASCII-coded hex
 */
#define RESPONSE_DATA_MAX_SIZE_HASCII (HEX_CHARS_PER_BYTE * RESPONSE_DATA_MAX_SIZE)

// This defines the length of a response footer in bytes
// There currently is no footer for debug port
#define RESPONSE_FOOTER_SIZE        (0)
/** This defines the length of a response header in ASCII-coded
 * hex.
 */
#define RESPONSE_FOOTER_SIZE_HASCII (HEX_CHARS_PER_BYTE * RESPONSE_FOOTER_SIZE)

// This defines the maximum response size in bytes
#define RESPONSE_MAX_SIZE        (RESPONSE_HEADER_SIZE + RESPONSE_DATA_MAX_SIZE + RESPONSE_FOOTER_SIZE)
/*This defines the maximum response size in ASCII-coded hex.
 */
#define RESPONSE_MAX_SIZE_HASCII (HEX_CHARS_PER_BYTE * RESPONSE_MAX_SIZE)

// This is the stop byte used for all outgoing responses.
#define RESPONSE_STOP_BYTE ('\r')

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

// Structure to hold the ASCII hex data when retrieving a valid command from the
// RX circular buffer
typedef struct
{
   // Denotes if the start byte has been found while searching for a complete message
   bool isStartByteFound;
   // Size of the data buffer
   uint16_t dataBufferLen;
   // Buffer used for storing the complete ASCII command during processing
   char data[COMMAND_MAX_SIZE_HASCII];
} ASCIICommandItem_t;

// Holds statistics on TX/RX data and messages
// Note this structure needs to be packed properly since it is reused in the GetSerialStatistics method
typedef struct
{
   // RX byte count for the port
   // Incremented after each call to UART_ReadByte
   uint32_t numBytesReceived;
   // TX byte count for the port
   // Incremented for when Data is sent to the Serial Protocol module
   uint32_t numBytesSent;
   // Number of complete message received
   // Incremented after a complete command is found during the
   // scheduled update loop
   uint32_t numMessagesReceived;
   /** The number of complete messages sent Incremented at each call
    * to the Send Response function
    */
   uint32_t numMessagesSent;
} TxRxStatistics_t;

// Structure to hold buffers and data for each port
typedef struct
{
   // This is the message structure for the message that must be
   // populated and sent to the message router for routing to the
   // destination software module.
   MessageRouter_Message_t currentMessage;

   // This buffer is allocated for command data and is pointed to by the
   // command proc message structure.
   uint16_t commandBuffer[COMMAND_DATA_MAX_SIZE];

   // This buffer is allocated for response data and is pointed to by the
   // command proc message structure.
   uint16_t responseBuffer[RESPONSE_DATA_MAX_SIZE];

   // This is the information for assembling the next command as we
   // dequeue bytes from the UART driver
   // The data in this buffer is ASCII data that must be converted to binary
   // before sending on to the command processor.
   ASCIICommandItem_t asciiCommand;

   /** The address for this device on this port For simplicity in
    * the driver, this initializes to BROADCAST_ADDRESS if
    * addressing is not used
    */
   uint16_t deviceAddress;

   // Stats for transmit and receive data
   TxRxStatistics_t statistics;
} PortData_t;

// This structure holds the private information for this module
typedef struct
{
    // Module Id given to this module at Initialization
    uint16_t moduleId;

    // Initialization state for the module
    bool isInitialized;

    // Enable state for the module
    bool enableState;

   // Create a status object for each port used
   PortData_t portData[UART_DRV_CHANNEL_COUNT];
} Serial_Status_t;

/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The variable used for holding all internal data for this module.
static Serial_Status_t status;

/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

/** Description:
 *    This function searches the circular buffer for the given channel to find
 *    a complete command to be processed. (All data between Start and Stop
 *    characters.) Note that it may take mutiple calls to this function to
 *    locate a complete message.
 * Parameters:
 *    channel : The enumerated channel value for which this function will search for a command.
 *    asciiCommand : The located command in ASCII format
 * Returns:
 *    bool: The result of the command search
 * Return Value List:
 *    true: Command found and placed into the ASCII command buffer for
 *    processing
 *    false: No command found
 * History:
 *    * Date: Function created (EJH)    
 *
 */
static bool FindNextCommand(const UART_Drv_Channel_t channel, ASCIICommandItem_t *const asciiCommand);

/** Description:
 *    This function packetizes the given message response as ASCII-coded hex data and add the
 *    data to the outgoing transmit buffer;
 * Parameters:
 *    channel : The enumerated channel value used for sending this message.
 *    message : A pointer to the Message Router object defining the message to be sent.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
static void SendResponseAsciiHex(const UART_Drv_Channel_t channel,
                                 MessageRouter_Message_t *const message);

/** Description:
 *    This function takes a character '0' - 'F' and converts it to its hex equivalent
 *    ('F' becomes 0x0F)
 * Parameters:
 *    charToConvert: The character to be to be converted.
 * Returns:
 *    uint16_t: The Hex representation of the given character.
 * History:
 *    * Date: Function created (EJH)    
 *
 */
static uint16_t ConvertAsciiHexCharToNumeric(const uint16_t charToConvert);

/** Description:
 *    This function converts a 16-bit value to an ASCII-coded hex string
 *    and stores it in the given buffer. NOTE: The given buffer must be as long
 *    as the specified length
 * Parameters:
 *    destinationBuffer: The pointer to the buffer where the conversion is to be stored
 *    desiredLength: The number of nibbles to be converted (Max 4)
 *    valueToConvert: The value that is to be converted
 * History:
 *    * Date: Function created (EJH)    
 *
 */
static void ConvertNumericToAsciiHexString(uint16_t *const destinationBuffer, const uint16_t desiredLength,
                                           const uint16_t valueToConvert);

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

// Search circular buffer for the next command
static bool FindNextCommand(const UART_Drv_Channel_t channel, ASCIICommandItem_t *const asciiCommand)
{
   // Start with no command found
   bool wasCommandFound = false;

   // Verify the channel is valid
   if ((asciiCommand != 0) && (channel < UART_DRV_CHANNEL_COUNT))
   {
      // Init to null char
      uint16_t tmpByte = 0U;

      // Get all bytes from the circular RX buffer
      // Note this reads from the buffer not the port so it does not block.
      //while ((!wasCommandFound) && (UART_Drv_ReadCharArray(channel, &tmpByte, 1)))
      while ((!wasCommandFound) && (tmpByte = UART_Drv_ReadChar(channel)))
      {
         // Increase the number of bytes received for this channel
         status.portData[channel].statistics.numBytesReceived += sizeof(tmpByte);

         // See if the current byte is a command "Start" byte.
         if (tmpByte == COMMAND_START_BYTE)
         {
            // Store the flag so if the buffer only contains the first half of the
            // message, we will continue next time.
            asciiCommand->isStartByteFound = true;
            // Always reset the size when a start byte is found.  If
            // the start byte of the next message is received before
            // the stop byte of the previous message, then the previous
            // message will be ignored.
            asciiCommand->dataBufferLen = 0;
         }
         else if ((tmpByte == COMMAND_STOP_BYTE_1) || (tmpByte == COMMAND_STOP_BYTE_2))
         {
            // Complete command found: clear start byte flag
            asciiCommand->isStartByteFound = false;
            // Mark that we have found a command (which will exit the loop)
            wasCommandFound = true;
         }
         else if (asciiCommand->isStartByteFound)
         {
            // If we have room for the next byte of the command...
            if (asciiCommand->dataBufferLen < (uint16_t)COMMAND_MAX_SIZE_HASCII)
            {
               // Add byte to command buffer and increment size
               asciiCommand->data[asciiCommand->dataBufferLen++] = tmpByte;
            }
            // Otherwise, clear the command buffer and send an error...
            else
            {
               // Reset command buffer
               asciiCommand->isStartByteFound = false;
               asciiCommand->dataBufferLen = 0;
            }
         }
         // else, byte is not part of a valid message.  Throw it away
      }
   }

   return(wasCommandFound);
}


// Send message response using hex encoding
static void SendResponseAsciiHex(const UART_Drv_Channel_t channel,
                                 MessageRouter_Message_t *const message)
{
   // Make sure the given channel is valid
   if (channel < UART_DRV_CHANNEL_COUNT)
   {
      // Check for NULL pointer
      if (message != 0)
      {
         // Message structure is valid.
         // Make sure the response data buffer is valid.
         if (message->responseParams.data != 0)
         {
            // Start with the CRC seed value
            uint16_t calculatedCRC = CRC_SEED;

            // Response data appears to be valid, so send the HASCII response.
            // Start Byte
            //uint16_t temp = (uint16_t)RESPONSE_START_BYTE;
            uint16_t temp = 0x3e;
            Serial_Send(channel, &temp, 1, SERIAL_ENCODING_BINARY);
				
#if (NUM_ADDRESS_BYTES > 0)
            // Send address - 0 is the master
            Serial_Send(channel, 0, 1, SERIAL_ENCODING_ASCII_CODED_HEX);
            calculatedCRC = CRCLib_Calculate(calculatedCRC, 0, 1);
#endif
            // ModID
            Serial_Send(channel, &message->header.moduleID, 1, SERIAL_ENCODING_ASCII_CODED_HEX);
            // CmdID
            Serial_Send(channel, &message->header.commandID, 1, SERIAL_ENCODING_ASCII_CODED_HEX);
            // MsgID
            Serial_Send(channel, &message->header.messageID, 1, SERIAL_ENCODING_ASCII_CODED_HEX);
            // Length
            Serial_Send(channel, &message->responseParams.length, 1, SERIAL_ENCODING_ASCII_CODED_HEX);
            // Buffer
            Serial_Send(channel, message->responseParams.data, message->responseParams.length, SERIAL_ENCODING_ASCII_CODED_HEX);


#if (NUM_CRC_BYTES > 0)
				// Calculate the CRC
            calculatedCRC = CRCLib_Calculate(calculatedCRC, &message->header.moduleID, 1);
            calculatedCRC = CRCLib_Calculate(calculatedCRC, &message->header.commandID, 1);
            calculatedCRC = CRCLib_Calculate(calculatedCRC, &message->header.messageID, 1);
            calculatedCRC = CRCLib_Calculate(calculatedCRC, &message->responseParams.length, 1);
            calculatedCRC = CRCLib_Calculate(calculatedCRC, message->responseParams.data, message->responseParams.length);
            // Send the CRC
            //Serial_Send(channel, (uint16_t *)&calculatedCRC, sizeof(calculatedCRC)*2, SERIAL_ENCODING_ASCII_CODED_HEX);
            Serial_Send(channel, (uint16_t *)&calculatedCRC, HEX_MULTIPLE * sizeof(calculatedCRC), SERIAL_ENCODING_ASCII_CODED_HEX);
            //Serial_Send(channel, (uint16_t *)&calculatedCRC, 1, SERIAL_ENCODING_ASCII_CODED_HEX);
            //Serial_Send(channel, (uint16_t *)&calculatedCRC, 1, SERIAL_ENCODING_ASCII_CODED_HEX);
#endif 

            // Stop Byte
            //temp = (uint16_t)RESPONSE_STOP_BYTE;
            temp = 13;
            Serial_Send(channel, &temp, 1, SERIAL_ENCODING_BINARY);

            // Increment the number of messages sent
            status.portData[channel].statistics.numMessagesSent++;
         }
      }
   }
}


// Convert a series of ASCII-coded hex values to a single 16-bit numeric value
uint16_t Serial_ConvertAsciiHexStringToNumeric(uint16_t *const hexCharacters, const uint16_t numHexCharacters)
{
   // Default to 0 result
   uint16_t resultValue = 0U;
   uint16_t tmpByte = '0';
   const uint16_t *nextByte;

   // Check for NULL pointer.  If it is NULL return 0 for the hex value.
   if (hexCharacters != 0)
   {
      // Point to the beginning of the data
      nextByte = hexCharacters;

      // Since this returns a uint16_t, we can only convert a maximum
      // of 4 characters.  If a length > 4 is specified, only do the
      // first 4 characters.
      uint16_t tmpNumHexCharacters = numHexCharacters;

      if (tmpNumHexCharacters > 4)
      {
         tmpNumHexCharacters = 4;
      }

      // Number of bits to shift
      uint16_t shiftValue = 0U;

      // Loop through each character
      // Use 1-based indexing
      for (uint16_t i = 1U; i <= tmpNumHexCharacters; i++)
      {
         // Convert next ASCII character to its HEX equivalent
         tmpByte = ConvertAsciiHexCharToNumeric(*nextByte);

         // We need to shift the HEX character to it's correct position:
         // (i.e.) 7 in 0xF7FF must be shifted left 8 positions from zero
         // to be in the right spot.
         shiftValue = 4 * (tmpNumHexCharacters - i);
         resultValue += ((uint16_t)tmpByte) << shiftValue;

         // Move to the next byte for the next pass through loop
         nextByte++;
      }
   }

   // Finally, return the converted value
   return(resultValue);
}


// Convert ASCII-coded hex value to 8-bit numeric representation
static uint16_t ConvertAsciiHexCharToNumeric(const uint16_t charToConvert)
{
   uint16_t value = 0U;

   // 'A' - 'F'
   if ((charToConvert > 0x40) && (charToConvert < 0x47))
   {
      value = (charToConvert - 0x37);
   }
   // 'a' - 'f'
   else if ((charToConvert > 0x60) && (charToConvert < 0x67))
   {
      value = (charToConvert - 0x57);
   }
   // '0' - '9'
   else if ((charToConvert > 0x2F) && (charToConvert < 0x3A))
   {
      value = (charToConvert - 0x30);
   }

   // else, invalid hex character -- return 0

   return(value);
}

//
static char GetHexNibble(const uint16_t valueToConvert, uint16_t nibbleIndex)
{
    char tmpASCIIChar;
    uint16_t tmpNibble = 0x0FU & (valueToConvert >> (4 *nibbleIndex));

    // Convert to ASCII
    // 0 - 1
    if (tmpNibble < 10)
    {
       tmpASCIIChar = tmpNibble + 0x30;
    }
    // A - F
    else if (tmpNibble < 16)
    {
       // character is 11-15 since it was masked to one nibble
       tmpASCIIChar = tmpNibble + 0x37;
    }
    else
    {
       // Added so that if statement has a default case
       tmpASCIIChar = 0;
    }

    return(tmpASCIIChar);
}
// Convert value to ASCII-coded hex
static void ConvertNumericToAsciiHexString(uint16_t *const destinationBuffer, const uint16_t desiredLength,
                                           const uint16_t valueToConvert)
{
   // Check for NULL pointer.
   if (destinationBuffer != 0)
   {
      // Loop through the number of characters requested and build the ASCII string
       for (int i = 0; i < desiredLength; i+=2)
       {
          // Put the ASCII value in the next buffer position
          // Destination is buffer address
          *(destinationBuffer + i + 0) = GetHexNibble(valueToConvert, i + 1);
          *(destinationBuffer + i + 1) = GetHexNibble(valueToConvert, i + 0);
          //*(destinationBuffer + 2) = GetHexNibble(valueToConvert, 3);
          //*(destinationBuffer + 3) = GetHexNibble(valueToConvert, 2);
       }
   }
}

/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


// Initialize all configured serial ports
bool Serial_Init(const uint32_t moduleId, const void *configData)
{
    // Default module to uninitialized and not enabled
    status.isInitialized = false;
    status.enableState = false;
    // Store the module Id for error reporting
    status.moduleId = moduleId;

    // TODO: Move configuration to table
    // For now, no configuration data is passed in to this module

    //-----------------------------------------------
    // Buffer Initialization
    //-----------------------------------------------

    for (uint16_t portIndex = 0; portIndex < UART_DRV_CHANNEL_COUNT; portIndex++)
    {
        // Init the port information and buffers
        memset(&status.portData[portIndex], 0, sizeof(PortData_t));

        // Always start with the broadcast address
        status.portData[portIndex].deviceAddress = BROADCAST_ADDRESS;
    }

    // Mark initialization is complete
    status.isInitialized = true;

   // Finally, return the result of the initialization
   return(status.isInitialized);
}


// Scheduled update loop for processing messages
void Serial_Update(void)
{
   // Loop through each port and check for new commands
   //for (uint16_t channel = 0U; channel < UART_DRV_CHANNEL_COUNT; channel++)
   for (uint16_t channel = UART_DRV_CHANNEL_DEBUG; channel <= UART_DRV_CHANNEL_DEBUG; channel++)
   {
#ifdef DEBUG_SEND_CONSTANT_DATA
      // Constantly send data during every update loop
      // This is just used for debugging serial port
       uint16_t tmpByte = 0x2a;
      Serial_Send(UART_DRV_CHANNEL_DEBUG, &tmpByte, 1, SERIAL_ENCODING_BINARY);
#endif

      //-----------------------------------------------
      // Process RX Data
      //-----------------------------------------------
      // Store the command object for easy access
      ASCIICommandItem_t *asciiCommand = &(status.portData[channel].asciiCommand);

      // Look for a valid command in the circular RX buffer
      if (FindNextCommand((UART_Drv_Channel_t)channel, asciiCommand))
      {
         // A complete command was received, now we need to populate the standard
         // message structure with the data in this command.

         // Store the message object for easy access
         MessageRouter_Message_t *const message = &(status.portData[channel].currentMessage);

         // Make sure the length of the command is at least long enough to
         // contain a complete HASCII command header.  The data in the Next Command
         // buffer is HASCII, so compare it to the HASCII length of the command header.
         if (asciiCommand->dataBufferLen >=
                ((uint16_t)COMMAND_HEADER_SIZE_HASCII + (uint16_t)COMMAND_FOOTER_SIZE_HASCII))
         {
            // Init the message to no error
            status.portData[channel].currentMessage.responseCode = POWER_MESSAGEROUTER_RESPONSE_CODE_None;

            //-----------------------------------------------
            // Parse Header
            //-----------------------------------------------

            // Populate the command header.  This tells the Message Router
            // how to route the command to the destination module.

            // We start at the first byte
            uint16_t tmpIndex = (uint16_t)0U;
            // 2 hex character per byte
            uint16_t tmpCharacterCount = (uint16_t)HEX_CHARS_PER_BYTE;

#if (NUM_ADDRESS_BYTES > 0)
            // Extract the Destination Address
            // Note size has been verified above to be at least Address +  Message Header + Data Length
            uint16_t destinationAddress = (uint16_t)AsciiToHex(&(asciiCommand->data[tmpIndex]), tmpCharacterCount);

            // Move to the next byte
            tmpIndex += tmpCharacterCount;
#else

            // Addressing is not used, just set address to the broadcast address (0xFF)
            uint16_t destinationAddress = (uint16_t)BROADCAST_ADDRESS;
#endif

            // Verify this message is intended for us
            // If addressing is not used, our address will be the broadcast address and the message is accepted
            if ((destinationAddress == BROADCAST_ADDRESS) ||
                (destinationAddress == status.portData[channel].deviceAddress))
            {
               // Extract everything and verify the CRC
#if (NUM_CRC_BYTES > 0)
               // Init CRC to seed value
               uint16_t calculatedCRC = CRC_SEED;
#endif

               // This message is for us, continue and extract the Module ID
               message->header.moduleID =
                                          (uint16_t)Serial_ConvertAsciiHexStringToNumeric(&(asciiCommand->data[tmpIndex]), tmpCharacterCount);

               // Move to the next byte for CMD ID
               tmpIndex += tmpCharacterCount;
               message->header.commandID =
                                           (uint16_t)Serial_ConvertAsciiHexStringToNumeric(&(asciiCommand->data[tmpIndex]), tmpCharacterCount);

               // Move to the next byte for MSG ID
               tmpIndex += tmpCharacterCount;
               message->header.messageID =
                                           (uint16_t)Serial_ConvertAsciiHexStringToNumeric(&(asciiCommand->data[tmpIndex]), tmpCharacterCount);

               //-----------------------------------------------
               // Initialize Command Buffer
               //-----------------------------------------------

               // Move to the next byte for DATA LENGTH
               tmpIndex += tmpCharacterCount;
               // Assign the command buffer
               message->commandParams.data = status.portData[channel].commandBuffer;
               // Set the max size to prevent other modules from overwriting the bounds of the data buffer.
               message->commandParams.maxLength = (uint16_t)COMMAND_DATA_MAX_SIZE;
               // Get the length byte
               message->commandParams.length =
                                               (uint16_t)Serial_ConvertAsciiHexStringToNumeric(&(asciiCommand->data[tmpIndex]), tmpCharacterCount);

               //-----------------------------------------------
               // Initialize Response Buffer
               //-----------------------------------------------

               // Setup the buffer for the response
               message->responseParams.data = status.portData[channel].responseBuffer;
               message->responseParams.maxLength = (uint16_t)RESPONSE_DATA_MAX_SIZE;
               message->responseParams.length = 0U;

#if (NUM_CRC_BYTES > 0)
               // Extract the CRC - Skip data characters
               tmpIndex += (HEX_CHARS_PER_BYTE * message->commandParams.length);
               //&(asciiCommand->data[(uint16_t)tmpIndex + HEX_CHARS_PER_BYTE * message->commandParams.length]),
               // TODO -- Compiler intrinsic - Conversion routine does not handle byte order?
               uint16_t messageCRC = 0;
               for (unsigned int i = 0; i < NUM_CRC_BYTES; i++)
               {
                   // Move to next byte
                   tmpIndex += tmpCharacterCount;
                   // Use instrinsic to store byte
                   __byte((unsigned int*)&messageCRC, i) = (uint16_t)Serial_ConvertAsciiHexStringToNumeric(
                                                                  &(asciiCommand->data[(uint16_t)tmpIndex]),
                                                                  tmpCharacterCount);
               }

               // Calculate the CRC
               calculatedCRC = CRCLib_Calculate(calculatedCRC, &message->header.moduleID, 1);
               calculatedCRC = CRCLib_Calculate(calculatedCRC, &message->header.commandID, 1);
               calculatedCRC = CRCLib_Calculate(calculatedCRC, &message->header.messageID, 1);
               calculatedCRC = CRCLib_Calculate(calculatedCRC, &message->commandParams.length, 1);
               calculatedCRC = CRCLib_Calculate(calculatedCRC, message->commandParams.data, message->commandParams.length);
#else
               // If not using the CRC, just set to seed value for comparison
               uint16_t messageCRC = CRC_SEED;
#endif

                //-----------------------------------------------
                // Verify computed CRC
                //-----------------------------------------------
                //TODO - if (messageCRC == calculatedCRC)
               if (true)
                {

                   //-----------------------------------------------
                   // Process Command
                   //-----------------------------------------------

                   // Increment the number of messages received since this command will at least generate some sort of
                   // response message
                   status.portData[channel].statistics.numMessagesReceived++;

                   // Verify the length
                   // The length in the command buffer is what was specified in the command
                   // and represents the number of hex bytes are in the data field after converting
                   // from HASCII.  The sNextCommand buffer is still in HASCII, so we need to
                   // convert the length in the command buffer to HASCII by multiplying by 2.
                   if (asciiCommand->dataBufferLen ==
                          (COMMAND_HEADER_SIZE_HASCII + (HEX_CHARS_PER_BYTE * message->commandParams.length) +
                           COMMAND_FOOTER_SIZE_HASCII))
                   {
                      // Length is correct.
                      // Now ensure the length is within the bounds of the data buffer
                      // before we convert the HASCII bytes to binary and copy them to the
                      // buffer.  This will prevent buffer overflow.
                      if (message->commandParams.length <= message->commandParams.maxLength)
                      {
                         // Convert each byte in the data field from HASCII to hex.
                         for (uint16_t i = 0U; i < message->commandParams.length; i++)
                         {
                            // Convert the next data byte from HASCII to hex and store in
                            // the command data buffer.
                             // See if we are on system using 16-bit chars
                             if (16 == CHAR_BIT)
                             {
                                 // Use compiler intrinsic to write to data buffer
                                 __byte((unsigned int*)status.portData[channel].commandBuffer, i) = Serial_ConvertAsciiHexStringToNumeric(&(asciiCommand->data[(uint16_t)COMMAND_HEADER_SIZE_HASCII + HEX_CHARS_PER_BYTE * i]), HEX_CHARS_PER_BYTE);
                             }
                             else
                             {
                                status.portData[channel].commandBuffer[i] = (uint16_t)Serial_ConvertAsciiHexStringToNumeric(
                                                                            &(asciiCommand->data[(uint16_t)COMMAND_HEADER_SIZE_HASCII + HEX_CHARS_PER_BYTE * i]),
                                                                            HEX_CHARS_PER_BYTE);
                             }
                         }

                         // Process message
                         MessageRouter_ProcessMessage(message);
                         // Increment the number of messages sent
                         // Send the response out the serial port.
                         SendResponseAsciiHex((UART_Drv_Channel_t)channel, message);
                      }
                      else
                      {
                         // The specified length is longer than our available command buffer size.
                         // Do not process this command, just send a response with the same
                         // header, with a length of 0 and no data.
                         SendResponseAsciiHex((UART_Drv_Channel_t)channel, message);
                      }
                   }
                   else
                   {
                       // The specified length is incorrect.
                       // Do not process this message, just send a response with the same
                       // header, with a length of 0 and no data.
                       SendResponseAsciiHex((UART_Drv_Channel_t)channel, message);
                   }
                }
                else
                {
                    // CRC Mismatch
                    // Do not process this message, just send a response with the same
                    // header, with a length of 0 and no data.
                    SendResponseAsciiHex((UART_Drv_Channel_t)channel, message);
                }
            } // Dst Address

             // Command has been processed, remove it.
             asciiCommand->dataBufferLen = 0U;
         }
      }
   }
}


// Clear statistics for the given port
void Serial_ResetStats(const UART_Drv_Channel_t channel)
{
   // Verify the port is valid
   if (channel < UART_DRV_CHANNEL_COUNT)
   {
      // Port is valid, reset everything to 0
      memset(&(status.portData[channel].statistics), 0, sizeof(TxRxStatistics_t));
   }
}


// Send encoded data to the given port
void Serial_Send(const UART_Drv_Channel_t channel, uint16_t *const data, const uint16_t dataLength,
                       const Serial_Encoding_t outputEncoding)
{
   // Verify the channel index is valid
   if (channel < UART_DRV_CHANNEL_COUNT)
   {
      // Make sure the length of the data we want to send will fit in the TX buffer -- max 255 bytes
      if ((data != 0) && (dataLength < (uint16_t)TX_BUFFER_SIZE))
      {
         // Output buffer for converting each byte to hex -- only used for Hex encoding
         uint16_t tmpOutputBuffer[HEX_MULTIPLE * HEX_CHARS_PER_BYTE];

         // If this message is a response message, convert the TxBuffer data into
         // ASCII encoded hex or HASCII (i.e. 0x0F = "0F").
         switch (outputEncoding)
         {
            case SERIAL_ENCODING_ASCII_CODED_HEX:
               // Convert each data byte to HASCII and put in TX Buffer
               // UART_Write implements a circular buffer so we do not have to wait

                // Handle single byte of data
                if (dataLength == 1)
                {
                    // Be sure to mask data since we are sending only a single byte from data
                    ConvertNumericToAsciiHexString(tmpOutputBuffer, HEX_MULTIPLE, 0x00FF & *(data));
                    // Add the two characters to the tx buffer.
                    UART_Drv_Write(channel, tmpOutputBuffer, HEX_MULTIPLE);
                    // Increment the bytes sent by 2 bytes
                    status.portData[channel].statistics.numBytesSent += HEX_MULTIPLE;
                }
                else if (dataLength > 0) // MAke sure we have data to send
                {
                    // Else, we are trying to send more than one byte
                    // TODO: Should we support byte length that are not a multiple of HEX_MULTIPLE?
                    //for (uint16_t i = 0U; i < dataLength/HEX_MULTIPLE; i++)
                    for (uint16_t i = 0U; i < dataLength; i++)
                    {
                       ConvertNumericToAsciiHexString(tmpOutputBuffer, HEX_MULTIPLE, 0x00FF & (__byte((unsigned int*)data, i)));
                       UART_Drv_Write(channel, tmpOutputBuffer, HEX_MULTIPLE);
                       status.portData[channel].statistics.numBytesSent += HEX_MULTIPLE;
                       /*
                       // If data is more than 1 byte, send as 4 bytes on 16-bit system
                       ConvertNumericToAsciiHexString(tmpOutputBuffer, sizeof(tmpOutputBuffer), *(data + i));

                       // Add the two/four characters to the tx buffer.
                       UART_Drv_Write(channel, tmpOutputBuffer, (sizeof(tmpOutputBuffer)));
                       // Increment the bytes sent by 2 bytes
                       status.portData[channel].statistics.numBytesSent += (sizeof(tmpOutputBuffer));
                       */
                   }
               }

               break;
            case SERIAL_ENCODING_BINARY:
            default:
               // Otherwise, data the specified message directly in the TX Buffer
                UART_Drv_Write(channel, data, dataLength);
               // Increase bytes sent by dataLength
               status.portData[channel].statistics.numBytesSent += dataLength;
               break;
         }
      }
   }
}


// Message Router function to reutrn statistics
void Serial_MessageRouter_GetSerialStatistics(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------

   // This structure defines the format of the command
   typedef struct
   {
      // UART channel index being requested
      uint16_t channelIndex;
   } Command_t;

   // This structure defines the format of the response
   typedef struct
   {
      // Items from Statistics_t
      uint32_t numBytesSent;
      uint32_t numBytesReceived;
      uint32_t numMessagesSent;
      uint32_t numMessagesReceived;
      uint32_t msSinceLastMessageReceived;
   } Response_t;

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //	Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), sizeof(Response_t)))
   {
      // Cast the command buffer as the command type
      Command_t *command = (Command_t *)message->commandParams.data;

      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Verify the index is valid
      if (command->channelIndex < UART_DRV_CHANNEL_COUNT)
      {
         // Port is valid, store the statistics object for easy access
         TxRxStatistics_t *tmpStatistics = &(status.portData[command->channelIndex].statistics);

         // Just store each of the items for the given port
         response->numBytesSent = tmpStatistics->numBytesSent;
         response->numBytesReceived = tmpStatistics->numBytesReceived;
         response->numMessagesSent = tmpStatistics->numMessagesSent;
         response->numMessagesReceived = tmpStatistics->numMessagesReceived;
         // TODO - Calculate time since last message
         response->msSinceLastMessageReceived = 0;
      }

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}
