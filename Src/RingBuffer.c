/*******************************************************************************
// Ring Buffer Library
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/

// Module Includes
#include "RingBuffer.h"
// Platform Includes
// Other Includes
#include <stdint.h>







static bool IsNonZeroPowerOfTwo(const uint32_t valueToCheck);
static bool IsNonZeroPowerOfTwo(const uint32_t valueToCheck)
{
    // (!(x & (x - 1)) && x)
    return (valueToCheck != 0) && ((valueToCheck & (valueToCheck - 1)) == 0);
}

bool RingBuffer_Init(RingBuffer_t *const ringBuffer, RingBuffer_Data_t *const buffer, const uint16_t powerOfTwoSize)
{
    //Assume failure until initialization is completed
    bool wasSuccessful = false;

   // Verify the given parameters
    // Power of Two function will also verify given size if not zero
   if (ringBuffer)
   {
       // Initialize the ring buffer indices
       ringBuffer->readIndex = 0;
       ringBuffer->writeIndex = 0;

       // Verify the buffer and given size are valid 
       // Because of the optimization, 
       if (buffer && (IsNonZeroPowerOfTwo(powerOfTwoSize)))
       {       
           // Buffer is valid, store it
           ringBuffer->buffer = buffer;
           // Size is non-zero and is power of two, store it
           ringBuffer->bufferSize = powerOfTwoSize;

           // Note the initialization was successfully completed
           wasSuccessful = true;
       }
       else
       {
           // Buffer is not valid, just make buffer null with size 0 to prevent use
           // Read/Write functions will verify length and buffer pointer before use
           ringBuffer->bufferSize = 0;
           ringBuffer->buffer = (void *)0;
       }
   }

   // Return the initialization success status
   return (wasSuccessful);

}

uint16_t RingBuffer_GetDataLength(RingBuffer_t *const ringBuffer)
{
    // Assume length 0 until successful calculation
    uint16_t calculatedLength = 0;

    // Verify the buffer pointer
    if (ringBuffer)
    {
        // Buffer pointer is valid - calculate length
        // To allow length claculation in a single step, we use an optimzation using AND to elimante sign information
        // Note that this requires the buffer size to be a power of 2
        // Ex:  => (WriteIndex=5 - ReadIndex=6) & (BufferSize=8 - 1)
        //      => [0 1 2 3 4 5 6 7] 
        //      => [          w r  ] 
        //      => [+ + + + + - + +]
        //      => (  5 - 6  ) & (  8 - 1  )
        //      => (    -1   ) & (    7    )
        //      => (11111111b) & (00000111b)
        //      => (00000111b) => 7
        calculatedLength = (ringBuffer->writeIndex - ringBuffer->readIndex) & (ringBuffer->bufferSize - 1);
    }

    // Return the calculated length (0 if buffer was invalid)
    return(calculatedLength);
}

bool RingBuffer_WriteChar(RingBuffer_t *const ringBuffer, const RingBuffer_Data_t data)
{
    // Assume failure until successful write
    bool wasSuccessful = false;

    // Verify buffer parameter and verify buffer has space available, verify data buffer pointer is valid 
    // Buffer is full at Size-1 since we do not allow write and read to be the same 
    // If buffer size is 0,  this will do nothing (Assumes buffer was set if size is valid)
    if ((ringBuffer) && (ringBuffer->buffer))
    {
        // See if we are at the maximum buffer size (size - 1)
        if (RingBuffer_GetDataLength(ringBuffer) >= (ringBuffer->bufferSize - 1))
        {
            // At max size, move read index to make room
            ringBuffer->readIndex = (ringBuffer->readIndex + 1) & (ringBuffer->bufferSize - 1);
        }

        // Write the new data at the current write index
        ringBuffer->buffer[ringBuffer->writeIndex] = data;

        // Update the write index to reflect the new positon after the read
        // See the GetDataLength() function for a detailed explanation of the AND operation
        // TODO: Verify atomic write of index
        ringBuffer->writeIndex = (ringBuffer->writeIndex + 1) & (ringBuffer->bufferSize - 1);

        // Write was successful
        wasSuccessful = true;
    }

    // Return the success state (false if buffer was invalid)
    return (wasSuccessful);
}

bool RingBuffer_ReadChar(RingBuffer_t *const ringBuffer, RingBuffer_Data_t *const data)
{
    // Assume failure until successful read
    bool wasSuccessful = false;

    // Verify given buffer and data parameters are valid, also verify data buffer pointer is valid
    // And see if the Buffer is full at Size-1 since we do not allow write and read to be the same 
    if (ringBuffer && (RingBuffer_GetDataLength(ringBuffer) > 0) && (ringBuffer->buffer))
    {
        // Buffer has data, get data at current read index and sotre in the given buffer
        *data = ringBuffer->buffer[ringBuffer->readIndex];

        // Update the write index to reflect the new positon after the read
        // See the GetDataLength() function for a detailed explanation of the AND operation
        // TODO: Verify atomic write of index
        ringBuffer->readIndex = (ringBuffer->readIndex + 1) & (ringBuffer->bufferSize - 1);

        // Read was successful
        wasSuccessful = true;
    }

    // Return the success state (false if buffers were invalid)
    return (wasSuccessful);
}


