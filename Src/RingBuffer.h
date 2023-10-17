/*******************************************************************************
// Ring Buffer Library
*******************************************************************************/
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
// Other Includes
#include <stdbool.h>
#include <stdint.h>

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

// Defines the type used by the ring buffer. This allows uint16_t to be used for TI DSP.
typedef uint16_t RingBuffer_Data_t;

// Structure that defines all parameters for managing a single circular buffer
// The type used for holding all data for a ring buffer
// Note that the data buffer must be set using Init() before use
typedef struct 
{
    // Data buffer that holds all data for the ring buffer
    RingBuffer_Data_t *buffer;
    // Size of the data buffer - must be power of two
    // Note this is the whole buffer size, not the amount of data currently in the buffer
    // One character is used to seperate read/write so that a full buffer won't have
    // read=write which would be indistinguishable from an empty buffer.
    uint16_t bufferSize;
    // The current read position: 0 to (size-1)
    // Note: Max is size-1 because we do not let read and write be equal
    uint16_t readIndex;
    // The current write position: 0 to (size-1)
    // Note: Max is size-1 because we do not let read and write be equal
    uint16_t writeIndex;
} RingBuffer_t;

/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/** Description:
 *    This function initializes the given circular buffer structure.  The given
 *    buffer will be set and the length stored.
 * Parameters:
 *    ringBuffer - A pointer to the ring buffer structure to be
 *      initialized.  Note that memory is not allocated by this function.
 *    buffer - The data buffer to be used by this ring buffer item
 *    powerOfTwoSize - The total size of given buffer. Note that the optimizations
 *      in this library require buffers to be a non-zero power of two (Ex. 2,8,16,128,...32768 [max])
 *      Note that the internal optimizations will only allow a max of (powerOfTwoSize-1) elements to be stored.
 *  Returns:
 *    bool - The result of the initialization
 *  Return Value List:
 *    true - The circular buffer structure was successfully initialized with the given parameters
 *    false - The circular buffer structure could not be initialized with the given parameters
 *    (Ex. invalid pointers or size is not a power of two)
 */
bool RingBuffer_Init(RingBuffer_t *const ringBuffer, RingBuffer_Data_t *const buffer, const uint16_t powerOfTwoSize);

/** Description:
 *    This function calculates the current number of elements
 *    currently in the given ring buffer structure.  To maintain
 *    atomicity, size is not stored and is always calculated
 *    based on the current read/write indices.
 *    Note that the maximum length is one less than the size of
 *    internal buffer due to internal optimizations.
 * Parameters:
 *    ringBuffer - A pointer to the ring buffer structure for
 *    which the number of elements is to be calculated.
 *  Returns:
 *    uint16_t - The number of elements currently in the given
 *    ring buffer. Returns 0, if buffer is invalid.
 */
uint16_t RingBuffer_GetDataLength(RingBuffer_t *const ringBuffer);


/** Description:
 *    Adds the given data to the end of internal FIFO for the
 *    given ring buffer (if space is available). If the buffer
 *    is full, no change is made. If successful, the internal
 *    indices are updated in the given structure.  The updates
 *    to read/write indices are atomic.
 * Parameters: 
 *    ringBuffer - Pointer to the ring buffer structure where
 *       the data is to be added.
 *    data -  New data to be added to the ring buffer.
 * Returns:
 *    bool - The result of the data addition.
 * Return Value List:
 *    true :   The given data was successfully added to the ring
 *             buffer.
 *    false :  The given data could not be stored in the ring
 *             buffer. Either the buffer is invalid or the
 *             buffer is currently full.  
 */
bool RingBuffer_WriteChar(RingBuffer_t *const ringBuffer, const RingBuffer_Data_t data);


/** Description:
 *    Retrieves the first (oldest) available byte from the
 *    internal FIFO of the given ring buffer. If
 *    successful, the internal indices are updated in the given
 *    structure.  The updates to read/write indices are atomic.
 * Parameters:
 *    ringBuffer - Pointer to the ring buffer structure
 *       from which the data is to be retrieved.
 *    data - Pointer where retrieved data will be stored.
 * Returns:
 *    bool -  The result of the dequeue operation
 * Return Value List:
 *    true - Data was dequeued successfully and stored in the
 *       given location
 *    false - No data was dequeued. Either no data was available
 *    or the given pointers were invalid.
 */
bool RingBuffer_ReadChar(RingBuffer_t *const ringBuffer, RingBuffer_Data_t *const data);


#ifdef __cplusplus
extern "C"
}
#endif
