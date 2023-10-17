/*******************************************************************************
// GPIO Extended Functions
// Defines a version of GPIO functions that act on two parameters (Port & Pin) 
// rather than a single pin. This prevent converting back and forth between 
// GPIO Pin numbers and Port Pin numbers. 
*******************************************************************************/

// Prevent multiple inclusion of header file
#pragma once

// Module Includes
// Platform Includes
// Other Includes
#include "gpio.h" // TI DriverLib
#include <stdbool.h>
#include <stdint.h>

// Start C Binding Section for C++ Compilers
#ifdef __cplusplus
extern "C"
{
#endif


/*******************************************************************************
// Public Constant Definitions
*******************************************************************************/

// Maximum of 32 pins in each port
// Can also use PORT_PIN_MASK_U16
#define GPIO_PINS_PER_PORT (32U)

// TMS320F28388D has GPIO Pins 0-168 (169 total)
#define GPIO_PIN_MAX (169U)

#define GPIO_LEVEL_LOW  (0U)

#define GPIO_LEVEL_HIGH (1U)

/*******************************************************************************
// Public Type Declarations
*******************************************************************************/


// Pin level used by TI DriverLib
// PORT_PIN_LEVEL_LOW or PORT_PIN_LEVEL_HIGH
typedef uint32_t GPIO_Level_t;


/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

// A port/pin combination of the TI DriverLib function
// Only checks to make sure we don't exceed the size of the largest package
//*****************************************************************************
//
//! \internal
//! Checks that a pin number is valid for a device.
//!
//! Note that this function reflects the highest possible GPIO number of a
//! device on its biggest package. Check the datasheet to see what the actual
//! range of valid pin numbers is for a specific package.
//!
//! \return None.
//
//*****************************************************************************

static inline bool GPIO_IsPortPinValid(GPIO_Port gpioPort, uint32_t portPin)
{
    // Max GPIO pin is GPIO168 (169 total GPIO pins)
    return(((gpioPort * GPIO_PINS_PER_PORT) + portPin) < GPIO_PIN_MAX);
}

// A port/pin combination of the TI DriverLib Read function
// This is provided to prevent converting back and forth between ports and pins
//*****************************************************************************
//
//! Reads the value present on the specified pin.
//!
//! \param pin is the identifying GPIO number of the pin.
//!
//! The value at the specified pin are read, as specified by \e pin. The value
//! is returned for both input and output pins.
//!
//! The pin is specified by its numerical value. For example, GPIO34 is
//! specified by passing 34 as \e pin.
//!
//! \return Returns the value in the data register for the specified pin.
//
//*****************************************************************************
static inline uint32_t GPIO_ReadPortPin(GPIO_Port gpioPort, uint32_t portPin)
{
    volatile uint32_t *gpioDataReg;

    // Check the arguments
    ASSERT(GPIO_IsPortPinValid(gpioPort, portPin));

    // Compute the read data register location
    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  (gpioPort * GPIO_DATA_REGS_STEP);

    return((gpioDataReg[GPIO_GPxDAT_INDEX] >> (portPin % GPIO_PINS_PER_PORT)) & (uint32_t)0x1U);
}

// A port/pin combination of the TI DriverLib Write function
// This is provided to prevent converting back and forth between ports and pins
//*****************************************************************************
//
//! Writes a value to the specified pin.
//!
//! \param pin is the identifying GPIO number of the pin.
//! \param outVal is the value to write to the pin.
//!
//! Writes the corresponding bit values to the output pin specified by
//! \e pin.  Writing to a pin configured as an input pin has no effect.
//!
//! The pin is specified by its numerical value. For example, GPIO34 is
//! specified by passing 34 as \e pin.
//!
//! \return None.
//
//*****************************************************************************
static inline void GPIO_WritePortPin(GPIO_Port gpioPort, uint32_t portPin, uint32_t level)
{
    volatile uint32_t *gpioDataReg;
    uint32_t pinMask;

    // Check the arguments similar to the TI DriverLib
    ASSERT(GPIO_IsPortPinValid(gpioPort, portPin));

    // Compute the set/clear data register location
    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  (gpioPort * GPIO_DATA_REGS_STEP);

    // Determine the mask in the given port
    pinMask = (uint32_t)1U << (portPin % GPIO_PINS_PER_PORT);

    // Set or clear the given pin
    if(GPIO_LEVEL_HIGH == level)
    {
        gpioDataReg[GPIO_GPxSET_INDEX] = pinMask;
    }
    else
    {
        gpioDataReg[GPIO_GPxCLEAR_INDEX] = pinMask;
    }
}

// A port/pin combination of the TI DriverLib Toggle function
// This is provided to prevent converting back and forth between ports and pins
//*****************************************************************************
//
//! Toggles the specified pin.
//!
//! \param pin is the identifying GPIO number of the pin.
//!
//! Writes the corresponding bit values to the output pin specified by
//! \e pin.  Writing to a pin configured as an input pin has no effect.
//!
//! The pin is specified by its numerical value. For example, GPIO34 is
//! specified by passing 34 as \e pin.
//!
//! \return None.
//
//*****************************************************************************
static inline void GPIO_TogglePortPin(GPIO_Port gpioPort, uint32_t portPin)
{
    volatile uint32_t *gpioDataReg;

    // Check the arguments
    ASSERT(GPIO_IsPortPinValid(gpioPort, portPin));

    // Compute the toggle data register location
    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  (gpioPort * GPIO_DATA_REGS_STEP);

    gpioDataReg[GPIO_GPxTOGGLE_INDEX] = (uint32_t)1U << (portPin % GPIO_PINS_PER_PORT);
}


//*****************************************************************************
//
//! Sets the pad configuration for the specified pin.
//!
//! \param pin is the identifying GPIO number of the pin.
//! \param pinType specifies the pin type.
//!
//! This function sets the pin type for the specified pin. The parameter
//! \e pinType can be the following values:
//!
//! - \b GPIO_PIN_TYPE_STD specifies a push-pull output or a floating input
//! - \b GPIO_PIN_TYPE_PULLUP specifies the pull-up is enabled for an input
//! - \b GPIO_PIN_TYPE_OD specifies an open-drain output pin
//! - \b GPIO_PIN_TYPE_INVERT specifies inverted polarity on an input
//!
//! \b GPIO_PIN_TYPE_INVERT may be OR-ed with \b GPIO_PIN_TYPE_STD or
//! \b GPIO_PIN_TYPE_PULLUP.
//!
//! The pin is specified by its numerical value. For example, GPIO34 is
//! specified by passing 34 as \e pin.
//!
//! \return None.
//
//*****************************************************************************
void GPIO_SetPortPinPadConfig(GPIO_Port gpioPort, uint32_t portPin, uint32_t pinType);

//*****************************************************************************
//
//! Sets the direction and mode of the specified pin.
//!
//! \param pin is the identifying GPIO number of the pin.
//! \param pinIO is the pin direction mode.
//!
//! This function configures the specified pin on the selected GPIO port as
//! either input or output.
//!
//! The parameter \e pinIO is an enumerated data type that can be one of the
//! following values:
//!
//! - \b GPIO_DIR_MODE_IN
//! - \b GPIO_DIR_MODE_OUT
//!
//! where \b GPIO_DIR_MODE_IN specifies that the pin is programmed as an input
//! and \b GPIO_DIR_MODE_OUT specifies that the pin is programmed as an output.
//!
//! The pin is specified by its numerical value. For example, GPIO34 is
//! specified by passing 34 as \e pin.
//!
//! \return None.
//
void GPIO_SetPortPinDirectionMode(GPIO_Port gpioPort, uint32_t portPin, GPIO_Direction pinIO);

//*****************************************************************************
//
//! Sets the qualification mode for the specified pin.
//!
//! \param pin is the identifying GPIO number of the pin.
//! \param qualification specifies the qualification mode of the pin.
//!
//! This function sets the qualification mode for the specified pin. The
//! parameter \e qualification can be one of the following values:
//! - \b GPIO_QUAL_SYNC
//! - \b GPIO_QUAL_3SAMPLE
//! - \b GPIO_QUAL_6SAMPLE
//! - \b GPIO_QUAL_ASYNC
//!
//! To set the qualification sampling period, use
//! GPIO_setQualificationPeriod().
//!
//! \return None.
//
//*****************************************************************************
void GPIO_SetPortPinQualificationMode(GPIO_Port gpioPort, uint32_t portPin, GPIO_QualificationMode qualification);

// End of C Binding Section
#ifdef __cplusplus
}
#endif


