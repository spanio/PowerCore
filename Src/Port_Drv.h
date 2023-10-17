/*******************************************************************************
// Port Driver
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once


/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Port_Drv_Config.h"
// Platform Includes
#include "GPIO_Device.h"
// Other Includes
#include <stdbool.h> // Defines C99 boolean type


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

#define PORT_PIN_TO_MASK(x) ((uint32_t)1U << ((x) % 32U))


/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

// Defines the type used for API calls to specify a port pin values
typedef GPIO_Level_t Port_Drv_Level_t;

// Defines the type used for API calls to specify a port bit mask
typedef uint32_t Port_Drv_Mask_t;

// Define the configuration structure used for initialization
typedef struct
{
    // The number of items defined by pinConfigArray - calculated by compiler
    uint32_t numConfigItems;
    // The configuration for each of the pins to be configured
    const struct Port_Drv_Data_s *pinConfigArray;
} Port_Drv_Config_t;


/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/*******************************************************************************
// Description:
//    Initialize all ports as defined in the given configuration structure.
//    This will configure direction and modes for each pin. If configured as GPIO,
//    the pins can be read (input and output) and written (output only).
//    If the pins uses an alternate mode, calss to read/write/toggle will
//    not be valid.
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
bool Port_Drv_Init(const uint32_t moduleID, const Port_Drv_Config_t *configData);

bool Port_Drv_IsPortPinValid(Port_Drv_PortId_t portId, Port_Drv_Port_Pin_t portPinIndex);

/*******************************************************************************
// Description:
//    Reads a pin on the given port and returns the current state.  This will reflect
//    the configured inversion settings configured for the channel.
// Parameters:
//    portId - The logical identifier of the port to be read
//    portPin - The index of the pin in the specified port
// Returns: 
//    bool - The current logical state of the specified channel 
// Return Value List: 
//    true: The specified pin is currently at the value configured as high
//    false: The specified pin is currently at the value configured as low
*******************************************************************************/
bool Port_Drv_ReadPortPin(Port_Drv_PortId_t portId, Port_Drv_Port_Pin_t portPin);

/*******************************************************************************
// Description:
//    Writes the given output state to specified pin.
//    Writing to a pin configured as an input has no effect.
// Parameters:
//    portId - The logical identifier of the port to be read
//    portPin - The index of the pin in the specified port
//    activeState - is the new logical state to be written to the pin.
//    (true: configured high level, false: configured low level)
// Returns: 
//    none 
*******************************************************************************/
void Port_Drv_WritePortPin(Port_Drv_PortId_t portId, Port_Drv_Port_Pin_t portPin, bool activeState);

/*******************************************************************************
// Description:
//    Toggles the value of the specified channel and returns the new value.
//    Writing to a channel configured as an input has no effect.
// Parameters:
//    portId - The logical identifier of the port to be read
//    portPin - The index of the pin in the specified port
// Returns:
//    bool - The updated logical state of the specified pin
// Return Value List:
//    true: The specified pin has been flipped low->high
//    false: The specified pin has been flipped high->low
*******************************************************************************/
bool Port_Drv_TogglePortPin(Port_Drv_PortId_t portId, Port_Drv_Port_Pin_t portPin);

/*******************************************************************************
// Description:
//    Reads the pin value(s) specified by the mask for the given port.
//    This will reflect the configured inversion settings configured for the channel.
// Parameters:
//    portId - The logical identifier of the port to be read
//    dataMask - The mask identifying each of the pins to be read on the given port.
// Returns: 
//    Port_Drv_Mask_t - The current value for each pins as a bit mask.
//    Each bit represents the current state (1: HIGH or 0: LOW) of each pin.
*******************************************************************************/
Port_Drv_Mask_t Port_Drv_ReadMaskedPort(Port_Drv_PortId_t portId, Port_Drv_Mask_t dataMask);

/*******************************************************************************
// Description:
//    Write the pin value(s) specified to a masked set of pins on the given port.
// Parameters:
//    portId - The logical identifier of the port to be written
//    dataMask - The mask identifying each of the pins to be written on the given port.
//    portData - The new value for each pins specified as a bit mask.
//    Each bit represents the current state (1: HIGH or 0: LOW) of each pin to be written.
// Returns: 
//    none
*******************************************************************************/
void Port_Drv_WriteMaskedPort(Port_Drv_PortId_t portId, Port_Drv_Mask_t dataMask, Port_Drv_Level_t portData);

/*******************************************************************************
// Description:
//    Toggle the pin value(s) for a set of pins pins on the given port.
// Parameters:
//    portId - The logical identifier of the port to be written
//    dataMask - The mask identifying each of the pins to be toggle on the given port.
//    portData - The new value for each pins specified as a bit mask.
//    Each bit represents the new state (1: HIGH or 0: LOW) of each pin to be written.
// Returns: 
//    portData - The new value after toggling for each of the pins specified as a bit mask.
//    Each bit represents the updated state (1: HIGH or 0: LOW) of each pin after toggling.
*******************************************************************************/
Port_Drv_Level_t Port_Drv_ToggleMaskedPort(Port_Drv_PortId_t portId, Port_Drv_Mask_t dataMask);

/*******************************************************************************
// Description:
//    Reads the pin values for all pins in the given port.
//    This will reflect the configured inversion settings configured for the channel.
// Parameters:
//    portId - The logical identifier of the port to be read
// Returns: 
//    Port_Drv_Level_t - The current value for each pins as a bit mask.
//    Each bit represents the current state (1: HIGH or 0: LOW) of each pin.
*******************************************************************************/
Port_Drv_Level_t Port_Drv_ReadPort(Port_Drv_PortId_t portId);

/*******************************************************************************
// Description:
//    Write the pin values for all pins in the given port.
// Parameters:
//    portId - The logical identifier of the port to be written
//    portData - The new value for each pins specified as a bit mask.
//    Each bit represents the current state (1: HIGH or 0: LOW) of each pin to be written.
// Returns: 
//    none
*******************************************************************************/
void Port_Drv_WritePort(Port_Drv_PortId_t portId, Port_Drv_Level_t portData);

/*******************************************************************************
// Description:
//    Toggle the pin values for all pins on the given port.
// Parameters:
//    portId - The logical identifier of the port to be written
// Returns: 
//    portData - The new value after toggling for each of the pins specified as a bit mask.
//    Each bit represents the updated state (1: HIGH or 0: LOW) of each pin after toggling.
*******************************************************************************/
Port_Drv_Mask_t Port_Drv_TogglePort(Port_Drv_PortId_t portId);


/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif



