/*******************************************************************************
// GPIO Driver 
// Defines the common, logical interface for interfacing with individual or 
// groups of digital I/O channels. Pins and Groups are specified using only their 
// enumerated logical IDs.   The Port Driver maintains configuration of the 
// underlying port data. 
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "GPIO_Drv_Config.h" // Defines channel identifiers
// Platform Includes
#include "MessageRouter.h"
// Other Includes
#include <stdbool.h> // Defines C99 boolean type
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

// Defines common type used for representing GPIO port data in API functions
typedef uint32_t GPIO_Drv_PortLevel_t;

// Common configuration structure passed to the module intialization function
// Data is generally defined in the board-specific configuation file
typedef struct
{
    // The number of items in the gpioConfigData - calcuated by compiler
    // Note that this must match GPIO_DRV_CHANNEL_ID_COUNT
    uint16_t numConfigItems;
    // GPIO configuration data
    const GPIO_Drv_ChannelConfig_t *channelConfigArray;

    // The number of items in the gpioGroupConfigData - calcuated by compiler
    // Note that this must match GPIO_DRV_GROUP_ID_COUNT
    uint16_t numGroupConfigItems;
    // Group configuration data
    const GPIO_Drv_GroupConfig_t *groupConfigArray;
} GPIO_Drv_Config_t;


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
//    true: The module was initialized successfully
//    false: The module was unable to complete successful initialization.
//    Calls to API functions in the module will have unxepcted results.
*******************************************************************************/
bool GPIO_Drv_Init(const uint32_t moduleID, const GPIO_Drv_Config_t *configData);

/*******************************************************************************
// Description:
//    Reads an input channel and returns the current state.  This will reflect
//    the configured inversion settings configured for the channel.
// Parameters:
//    channelId - The logical identifier of the channel to be read
// Returns: 
//    bool - The current logical state of the specified channel 
// Return Value List: 
//    true: The specified channel is currently at the value configured as high
//    false: The specified channel is currently at the value configured as low
*******************************************************************************/
bool GPIO_Drv_ReadChannel(const GPIO_Drv_ChannelId_t channelId);

/*******************************************************************************
// Description:
//    Writes the given output state to specified channel.
//    Writing to a channel configured as an input has no effect.
// Parameters:
//    channelId - logical identifier of the channel to be written
//    activeState - is the new logical state to be written to the channel.
//    (true: configured high level, false: configured low level)
// Returns: 
//    none 
*******************************************************************************/
void GPIO_Drv_WriteChannel(const GPIO_Drv_ChannelId_t channelId, const bool activeState);

/*******************************************************************************
// Description:
//    Toggles the value of the specified channel and returns the new value.
//    Writing to a channel configured as an input has no effect.
// Parameters:
//    channelId - logical identifier of the channel to be toggled
//    activeState - is the new logical state to be written to the channel.
//    (true: configured high level, false: configured low level)
// Returns:
//    bool - The updated logical state of the specified channel
// Return Value List:
//    true: The specified channel has been flipped low->high
//    false: The specified channel has been flipped high->low
*******************************************************************************/
bool GPIO_Drv_ToggleChannel(const GPIO_Drv_ChannelId_t channelId);

/*******************************************************************************
// Description:
//    Reads a group of GPIO pins and returns the current state.  This will reflect
//    the configured inversion settings configured for the pins.
// Parameters:
//    groupId - The logical identifier of the channel to be read
// Returns: 
//    GPIO_Drv_PortLevel_t - A bit mask for all pins in the given GPIO group.
//    Each bit represents the current state (1: HIGH or 0: LOW) of each pin.
//    The value is adjusted for the group's specified offset and mask.
//    Ex. Mask 0xF0F0 with offset 4 would return 0x0F0F if all pins are set.
//    Note that both input and output pins configured for GPIO can be read.
*******************************************************************************/
GPIO_Drv_PortLevel_t GPIO_Drv_ReadChannelGroup(const GPIO_Drv_GroupId_t groupId);

/*******************************************************************************
// Description:
//    Writes the given level mask to the specified group of GPIO pins.
//    Writing to a pins configured as inputs will have no effect.
// Parameters:
//    channelId - logical identifier of the group to be written
//    level - A bit mask for all pins in the given GPIO group.
//    Each bit represents the current state (1: HIGH or 0: LOW) of each pin.
// Returns: 
//    none 
*******************************************************************************/
void GPIO_Drv_WriteChannelGroup(const GPIO_Drv_GroupId_t channelGroupId, const GPIO_Drv_PortLevel_t level);

/*******************************************************************************
// Description:
//    Toggles all pins in the specified group of GPIO pins.
//    Writing to a pins configured as inputs will have no effect.
// Parameters:
//    channelId - logical identifier of the group to be flipped
// Returns: 
// Returns: 
//    GPIO_Drv_PortLevel_t - A bit mask for all pins in the given GPIO group.
//    Each bit represents the current state (1: HIGH or 0: LOW) of each pin.
//    The value is adjusted for the group's specified offset and mask.
*******************************************************************************/
GPIO_Drv_PortLevel_t GPIO_Drv_ToggleChannelGroup(const GPIO_Drv_GroupId_t channelGroupId);


void GPIO_Drv_MessageRouter_GetActiveState(MessageRouter_Message_t *const message);

void GPIO_Drv_MessageRouter_SetActiveState(MessageRouter_Message_t *const message);

void GPIO_Drv_MessageRouter_ToggleActiveState(MessageRouter_Message_t *const message);

/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif


