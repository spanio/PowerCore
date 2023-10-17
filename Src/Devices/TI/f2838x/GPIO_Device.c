/*******************************************************************************
// GPIO Driver - TI F2838x Implementation
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "GPIO_Device.h"
// Platform Includes
// Other Includes
#include "gpio.h" // DriverLib GPIO API
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

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
void GPIO_SetPortPinPadConfig(GPIO_Port gpioPort, uint32_t portPin, uint32_t pinType)
{
    volatile uint32_t *gpioBaseAddr;
    uint32_t pinMask;

    // Check the arguments similar to the TI DriverLib
    ASSERT(GPIO_IsPortPinValid(gpioPort, portPin));

    gpioBaseAddr = (uint32_t *)GPIOCTRL_BASE +
                   (gpioPort * GPIO_CTRL_REGS_STEP);
    pinMask = (uint32_t)1U << (portPin % 32U);

    EALLOW;

    //
    // Enable open drain if necessary
    //
    if((pinType & GPIO_PIN_TYPE_OD) != 0U)
    {
        gpioBaseAddr[GPIO_GPxODR_INDEX] |= pinMask;
    }
    else
    {
        gpioBaseAddr[GPIO_GPxODR_INDEX] &= ~pinMask;
    }

    //
    // Enable pull-up if necessary
    //
    if((pinType & GPIO_PIN_TYPE_PULLUP) != 0U)
    {
        gpioBaseAddr[GPIO_GPxPUD_INDEX] &= ~pinMask;
    }
    else
    {
        gpioBaseAddr[GPIO_GPxPUD_INDEX] |= pinMask;
    }

    //
    // Invert polarity if necessary
    //
    if((pinType & (uint32_t)GPIO_PIN_TYPE_INVERT) != 0U)
    {
        gpioBaseAddr[GPIO_GPxINV_INDEX] |= pinMask;
    }
    else
    {
        gpioBaseAddr[GPIO_GPxINV_INDEX] &= ~pinMask;
    }

    EDIS;
}

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
void GPIO_SetPortPinDirectionMode(GPIO_Port gpioPort, uint32_t portPin, GPIO_Direction pinIO)
{
    volatile uint32_t *gpioBaseAddr;
    uint32_t pinMask;

    // Check the arguments similar to the TI DriverLib
    ASSERT(GPIO_IsPortPinValid(gpioPort, portPin));

    gpioBaseAddr = (uint32_t *)GPIOCTRL_BASE +
                   (gpioPort * GPIO_CTRL_REGS_STEP);
    pinMask = (uint32_t)1U << (portPin % 32U);

    EALLOW;

    //
    // Set the data direction
    //
    if(pinIO == GPIO_DIR_MODE_OUT)
    {
        //
        // Output
        //
        gpioBaseAddr[GPIO_GPxDIR_INDEX] |= pinMask;
    }
    else
    {
        //
        // Input
        //
        gpioBaseAddr[GPIO_GPxDIR_INDEX] &= ~pinMask;
    }

    EDIS;
}

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
void GPIO_SetPortPinQualificationMode(GPIO_Port gpioPort, uint32_t portPin, GPIO_QualificationMode qualification)
{
    volatile uint32_t *gpioBaseAddr;
    uint32_t qSelIndex;
    uint32_t shiftAmt;

    // Check the arguments similar to the TI DriverLib
    ASSERT(GPIO_IsPortPinValid(gpioPort, portPin));

    gpioBaseAddr = (uint32_t *)GPIOCTRL_BASE +
                   (gpioPort * GPIO_CTRL_REGS_STEP);
    shiftAmt = (uint32_t)GPIO_GPAQSEL1_GPIO1_S * (portPin % 16U);
    qSelIndex = GPIO_GPxQSEL_INDEX + ((portPin % 32U) / 16U);

    //
    // Write the input qualification mode to the register.
    //
    EALLOW;

    gpioBaseAddr[qSelIndex] &= ~((uint32_t)GPIO_GPAQSEL1_GPIO0_M << shiftAmt);
    gpioBaseAddr[qSelIndex] |= (uint32_t)qualification << shiftAmt;

    EDIS;
}

