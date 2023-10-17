/*******************************************************************************
 // UART Driver
 // Defines the common, logical interfaces for UART channels.
 *******************************************************************************/

#include <stdint.h>  // Define C99 integer types
#include <stddef.h>
#include <stdbool.h>

// Module Include
#include "UART_Drv.h"
#include "UART_Drv_Config.h"
#include "UART_Drv_ConfigTypes.h"
// Platform Includes
#include "driverlib.h"
#include "device.h"
#include "NonSafety/Lib/RingBuffer.h"


#define LOG_ERROR 0

#define UART_DRV_FIFO_TX_SIZE 16
#define UART_DRV_FIFO_RX_SIZE 16

// Size of the local ring buffer used for transmitting data.
// Note the buffer library requires this to be a power of two.
#define TX_BUFFER_SIZE (128)

// Size of the local ring buffer used for receiving data.
// Note the buffer library requires this to be a power of two.
#define RX_BUFFER_SIZE (128)

// This defines the maximum length of command data in bytes.
#define COMMAND_DATA_MAX_SIZE (48)

// This defines the maximum length of response data in bytes
#define RESPONSE_DATA_MAX_SIZE (48)



typedef struct
{
    GPIO_CoreSelect pinCpu;
    uint32_t pinTx;
    uint32_t pinTxCfg;
    uint32_t pinRx;
    uint32_t pinRxCfg;

    uint32_t interruptNumberTx;
    void (*handlerIsrTx)(void);
    uint32_t interruptNumberRx;
    void (*handlerIsrRx)(void);
    uint16_t interruptGroup;

} UART_PeriphCfg_t;

typedef struct
{
    uint16_t *buf;
    uint16_t size;

    uint16_t idxWrite;
    uint16_t idxRead;
    uint16_t cntChars;
    uint16_t cntOverrun;
} UART_Fifo_t;

typedef struct
{
    uint16_t sciaTx;
    uint16_t sciaRx;

    uint16_t scibTx;
    uint16_t scibRx;

    uint16_t scicTx;
    uint16_t scicRx;

    uint16_t scidTx;
    uint16_t scidRx;
} UART_ISR_Counters_t;

typedef struct
{
    uint16_t fifoTxBuf[UART_DRV_FIFO_TX_SIZE];
    uint16_t fifoRxBuf[UART_DRV_FIFO_RX_SIZE];
    UART_Fifo_t fifoTx;
    UART_Fifo_t fifoRx;
} UART_Drv_Channel_Fifo_t;

// Structure to hold the circular buffers for each port
typedef struct
{
    /*Defines all parameters for the RX circular buffer.
    */
    RingBuffer_t rxCircularBuffer;

    // RX data buffer.
    RingBuffer_Data_t rxCircularBufferData[RX_BUFFER_SIZE];

    //Defines all parameters for the TX circular buffer.
    RingBuffer_t txCircularBuffer;

    // TX data buffer.
    RingBuffer_Data_t txCircularBufferData[TX_BUFFER_SIZE];
} PortBuffers_t;



typedef struct
{
    uint32_t numConfigItems;
    //UART_Drv_ConfigSettings_s *configSettings;
    const UART_Drv_Config_t *configSettings;
} Local_Config_t;

// This structure holds the private information for this module
typedef struct
{
    const UART_Drv_Config_t *uartConfig;

    // Create one buffer object for each UART port used
    PortBuffers_t portBuffers[UART_DRV_CHANNEL_COUNT];
} UART_Status_t;


/*******************************************************************************
 // Private Variable Definitions
 *******************************************************************************/

// Status structure for this module
static UART_Status_t status;


/*******************************************************************************
 // Private Function Declarations
 *******************************************************************************/


/*******************************************************************************
 // Private Data Declarations
 *******************************************************************************/

static bool initDone = false;


/*******************************************************************************
 // Public Function Declarations
 *******************************************************************************/


/*******************************************************************************
 // Description:
 //    Returns the number of characters that are available to be read from
 //    the UART peripheral.
 //    // An invalid channel will return 0.
 // Parameters:
 //    channelId - The logical identifier of the channel to be read
 // Returns:
 //    uint16_t - The number of bytes read from the specified channel
 // Return Value List:
 //    0: No data is availble to be read.
 //    1+: One or more characters are availble to be read.
 *******************************************************************************/
uint16_t UART_Drv_GetNumCharsRX(const UART_Drv_Channel_t channelId) {
    uint16_t numChars = 0;

    if (initDone && (channelId < UART_DRV_CHANNEL_COUNT))
    {
        // Read status from FIFO
        numChars = SCI_getRxFIFOStatus(status.uartConfig->dataPtr[channelId].uartBase);
    }

    return numChars;
}

/*******************************************************************************
 // Description:
 //    Returns the number of characters that can be succesfully written to the
 //    UART peripheral for outgoing transmission.
 //    An invalid channel will return 0.
 // Parameters:
 //    channelId - The logical identifier of the channel to be written
 // Returns:
 //    uint16_t - The number of bytes written to the specified channel
 // Return Value List:
 //    0: The UART peripheral is busy and has no room available.
 //    1+: The maximum number of characters that can be written to perhiperal
 //    for transmission.
 *******************************************************************************/
uint16_t UART_Drv_GetNumCharsTX(const UART_Drv_Channel_t channelId) {
    if (initDone && (channelId < UART_DRV_CHANNEL_COUNT))
    {
        return (16U - SCI_getTxFIFOStatus(status.uartConfig->dataPtr[channelId].uartBase));
    } else
    {
        return (0);
    }
}


uint16_t UART_Drv_ReadPort(const UART_Drv_Channel_t channelId) {
    uint16_t newCharacter = 0;

    if (initDone &&  (channelId < UART_DRV_CHANNEL_COUNT))
    {
        if (SCI_getRxFIFOStatus(status.uartConfig->dataPtr[channelId].uartBase) > 0)
        {
            newCharacter =  SCI_readCharNonBlocking(status.uartConfig->dataPtr[channelId].uartBase);
        }
    }

    return (newCharacter);
}

uint16_t UART_Drv_ReadChar(const UART_Drv_Channel_t channelId) {
    uint16_t newCharacter = 0;

    if (initDone &&  (channelId < UART_DRV_CHANNEL_COUNT))
    {
        RingBuffer_ReadChar(&(status.portBuffers[channelId].rxCircularBuffer), &newCharacter);
    }

    return (newCharacter);
}

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
uint16_t UART_Drv_WriteChar(const UART_Drv_Channel_t channelId, uint16_t data) {
    uint16_t charsWritten = 0;

    if (initDone && (channelId < UART_DRV_CHANNEL_COUNT))
    {
        // Write directly to the TX FIFO
        //SCI_writeCharNonBlocking(status.uartConfig->dataPtr[channelId].uartBase, data);
        SCI_writeCharBlockingFIFO(status.uartConfig->dataPtr[channelId].uartBase, data);

        // Function does not return a value, assume we successful wrote a character if we made it this far
        charsWritten = 1;

        Interrupt_enable(INT_SCIA_TX);
    }

    return (charsWritten);
}

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
uint16_t UART_Drv_WriteCharArray(const UART_Drv_Channel_t channelId,
                                 uint16_t *const data, const uint16_t dataLength) {
    uint16_t numCharsWritten = 0;
    uint16_t fifoSize = 0;

    if (initDone && (channelId < UART_DRV_CHANNEL_COUNT))
    {
        // Calculate the number of bytes we can write to the TX FIFO
        fifoSize = 16U - SCI_getTxFIFOStatus(status.uartConfig->dataPtr[channelId].uartBase);
        if (dataLength < fifoSize)
        {
            fifoSize = dataLength;
        }
        for (uint16_t i = 0; i < fifoSize; i++)
        {
            // Write directly to the TX FIFO
            SCI_writeCharNonBlocking(status.uartConfig->dataPtr[channelId].uartBase, data[i]);
        }

        // Function does not return a value, assume we successful wrote a character if we made it this far
        numCharsWritten = fifoSize;
    }

    return (numCharsWritten);
}

// Write data to the given UART
void UART_Drv_Write(const UART_Drv_Channel_t channel, uint16_t *const data, const uint16_t length) {
    // Verify the given channel
    if (channel < UART_DRV_CHANNEL_COUNT)
    {
        // Validate the given buffer
        if (data != 0)
        {
            // Loop through the given data and add to the circular buffer
            for (uint16_t i = 0U; i < length; i++)
            {
                // Put given byte in TX buffer
                RingBuffer_WriteChar(&(status.portBuffers[channel].txCircularBuffer), *(data + i));
            }

            // See if there is data to send and we are idle
            if ((RingBuffer_GetDataLength(&(status.portBuffers[channel].txCircularBuffer)) > 0) && (UART_Drv_GetNumCharsTX(channel) > 0))
            {
                // TODO - IRQ not currently used
                // Just trigger the TX Callback so all sending goes through the same mechanism
                // First enable the TX event
                // Interrupt_enable(localUartConfig->channelConfigArray[channel].txIRQConfig.interruptNumber);
                // Then trigger the transmit buffer interrupt?

                // Variable where new byte is stored
                uint16_t tmpByte = 0;

                // Trigger the first write
                for (int i = 0; i < 16; i++)
                {
                    if (RingBuffer_ReadChar(&(status.portBuffers[channel].txCircularBuffer), &tmpByte))
                    {
                        // Byte successfully dequeued from the TX buffer, send it
                        // TODO Check is full?
                        if (!UART_Drv_WriteChar(channel, tmpByte))
                        {
                            break;
                        }
                    } else
                    {
                        break;
                    }
                }
            }
        }
    }
}

// This function is used to determine actual baud rate used and determine the error for the given clock configuration
// Value is 0 if channel is invalid or not configured
uint32_t UART_Drv_GetActualBaudRate(UART_Drv_Channel_t channelId) {
    // Default to 0 for unconfigured/invalid channel
    uint32_t baud = 0U;

    // Verify the given channel
    if (initDone && (channelId < UART_DRV_CHANNEL_COUNT))
    {
        // Channel is valid, fetch the base address for the given channel
        uint32_t base = status.uartConfig->dataPtr[channelId].uartBase;

        // Clear configuration -- will be overwritten by successful call to SCI_getConfig()
        uint32_t configMask = 0;

        // Clock is either call to SysCtl_getLowSpeedClock(SYS_OSCSRC_FREQ) or defined value SYS_LSPCLK_FREQ
        // Call the clock function since this is used for verifying the configuration
        // TODO - Should this be define or use Sys function?
        uint32_t lspclkHz = SysCtl_getLowSpeedClock(DEVICE_OSCSRC_FREQ);

        // Fetch the configuration for the peripheral -- note that the low speed clock must be passed in for the baud to be calculated correctly
        SCI_getConfig(base, lspclkHz, &baud, &configMask);
    }

    // Return the baud rate
    return (baud);
}

// initSCIAFIFO - Configure SCIA FIFO
void initSCIFIFO(const UART_Drv_Channel_t channelId) {
    if (channelId < UART_DRV_CHANNEL_COUNT)
    {
        // Interrupts that are used in this example are re-mapped to
        // ISR functions found within this file.
        //  INT_SCIA_RX
        //Interrupt_register(status.uartConfig->dataPtr[channelId].rxIRQConfig.interruptNumber, sciaRXFIFOISR);
#if 0
        Interrupt_register(INT_SCIA_RX, sciaRXFIFOISR);
        Interrupt_register(INT_SCIA_TX, sciaTXFIFOISR);
#endif
        //
        // 8 char bits, 1 stop bit, no parity. Baud rate is 9600.
        //
        // TODO - Should clock be define or use Sys function?
        SCI_setConfig(status.uartConfig->dataPtr[channelId].uartBase, SysCtl_getLowSpeedClock(DEVICE_OSCSRC_FREQ), status.uartConfig->dataPtr[channelId].baudRate, (SCI_CONFIG_WLEN_8 |
                                                                                                                                                                     SCI_CONFIG_STOP_ONE |
                                                                                                                                                                     SCI_CONFIG_PAR_NONE));
        SCI_enableModule(status.uartConfig->dataPtr[channelId].uartBase);
        //SCI_enableLoopback(status.configSettings->configSettings[channelId].uartBase);
        SCI_resetChannels(status.uartConfig->dataPtr[channelId].uartBase);
        SCI_enableFIFO(status.uartConfig->dataPtr[channelId].uartBase);

#if ISR
        // RX and TX FIFO Interrupts Enabled
        SCI_enableInterrupt(status.uartConfig->dataPtr[channelId].uartBase, (SCI_INT_RXFF | SCI_INT_TXFF));
        SCI_disableInterrupt(status.uartConfig->dataPtr[channelId].uartBase, SCI_INT_RXERR);
#endif
        //SCI_setFIFOInterruptLevel(channelConfig->uartBase, SCI_FIFO_TX16, SCI_FIFO_RX16);
        SCI_setFIFOInterruptLevel(status.uartConfig->dataPtr[channelId].uartBase, SCI_FIFO_TX16, SCI_FIFO_RX16);
        SCI_performSoftwareReset(status.uartConfig->dataPtr[channelId].uartBase);

        SCI_resetTxFIFO(status.uartConfig->dataPtr[channelId].uartBase);
        SCI_resetRxFIFO(status.uartConfig->dataPtr[channelId].uartBase);

#ifdef AUTOBAUD
        //
        // Perform an autobaud lock.
        // SCI expects an 'a' or 'A' to lock the baud rate.
        //
        SCI_lockAutobaud(status.uartConfig->configSettings[channelId].uartBase);
#endif
    }
}

bool UART_Drv_Init(const uint32_t moduleId, const UART_Drv_Config_t *configPtr)
{
    // Configure the pins specified by the board configuration
    // First, validate the given parameter is valid
    if ((configPtr) && (configPtr->dataPtr))
    {
        // Store the configuration data for later use
        status.uartConfig = configPtr;

        // Loop through the UART configuration and configure each channel
        for (uint32_t channelId = 0; channelId < status.uartConfig->numConfigItems; channelId++)
        {
            // Buffer Config ---
            // Store the buffer object for easy access
            PortBuffers_t *portBuffer = &(status.portBuffers[channelId]);

            // Initialize the Circular TX Buffer
            RingBuffer_Init(&(portBuffer->txCircularBuffer), portBuffer->txCircularBufferData, sizeof(portBuffer->txCircularBufferData));
            // Initialize the Circular RX Buffer
            RingBuffer_Init(&(portBuffer->rxCircularBuffer), portBuffer->rxCircularBufferData, sizeof(portBuffer->rxCircularBufferData));

            // UART Config---

            // TODO
            // Enable the SCI peripheral
            SysCtl_enablePeripheral(status.uartConfig->dataPtr[channelId].peripheral);

#if ISR
            // Interrupts that are used in this example are re-mapped to
            // ISR functions found within this file.
            Interrupt_register(status.uartConfig->dataPtr[channelId].rxIRQConfig.interruptNumber, sciaRXFIFOISR);
            Interrupt_register(status.uartConfig->dataPtr[channelId].txIRQConfig.interruptNumber, sciaTXFIFOISR);
#endif
            // Configure SCI Channel to use FIFO with interrupts
            initSCIFIFO((UART_Drv_Channel_t)channelId);

#if ISR
            // Enable the SCI TX/RX interrupts
            Interrupt_enable(status.uartConfig->dataPtr[channelId].rxIRQConfig.interruptNumber);
#endif

#if ISR
            // Clear the PIE Group
            // TODO: Convert to group from interrupt number
            Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
#endif
            initDone = true;
        }
    }

    return (true);
}


void UART_Drv_Update(void) {

    // Read any waiting data
    // This handles slower data if the FIFO is not yet full

    // Handle RX Error -- must reset peripheral
    uint16_t rxStatus = SCI_getRxStatus(SCIA_BASE);
    //SCI_clear
    if ((SCI_RXSTATUS_ERROR & rxStatus))
    {
        //while(1);
        // TODO: PERFORM SW RESET INSTEAD?
        SCI_performSoftwareReset(SCIA_BASE);
        //SCI_resetChannels(SCIA_BASE);
    }

    // Polled UART to avoid ISRs on main core
    // Read RC FIFIO characters into ring buffer -- old data is discarded
    while (UART_Drv_GetNumCharsRX(UART_DRV_CHANNEL_DEBUG))
    {
        uint16_t newChar = SCI_readCharNonBlocking(SCIA_BASE);
        RingBuffer_WriteChar(&(status.portBuffers[UART_DRV_CHANNEL_DEBUG].rxCircularBuffer), newChar);
    }

    // Get next byte data from circular buffer, if any
    uint16_t newChar;
    while ((SCI_getTxFIFOStatus(SCIA_BASE) < SCI_FIFO_TX16) && (RingBuffer_ReadChar(&(status.portBuffers[UART_DRV_CHANNEL_DEBUG].txCircularBuffer), &newChar)))
    {
        // Byte successfully dequeued from the TX buffer, send it
        //UART_Drv_WriteChar(UART_DRV_CHANNEL_DEBUG, tmpByte);
        SCI_writeCharNonBlocking(SCIA_BASE, newChar);
    }
}

