/*******************************************************************************
// Port Driver - TI F2838xD Implementation
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include <driverlib.h> // TI DriverLib
#include "Port_Drv.h"
#include "Port_Drv_Config.h"
#include "Port_Drv_ConfigTypes.h"
// Platform Includes
#include "GPIO_Device.h"
// Other Includes
#include <stdbool.h> // Defines C99 boolean type
#include <stdint.h> // Defines C99 integer types

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

// DriverLib defines ports A-F
// Note: Don't confuse this with Port Channel
#define GPIO_PORT_COUNT_U16 ((uint16_t)(GPIO_PORT_F - GPIO_PORT_A + 1U))

// Mask used for pin data - Bits 0..31
#define PORT_PIN_MASK_U32 ((uint32_t)0xFFFFFFFFUL)
#define PORT_A_PIN_MASK_U32 ((uint32_t)0xFFFFFFFFUL)
#define PORT_B_PIN_MASK_U32 ((uint32_t)0xFFFFFFFFUL)
#define PORT_C_PIN_MASK_U32 ((uint32_t)0xFFFFFFFFUL)
#define PORT_D_PIN_MASK_U32 ((uint32_t)0xFFFFFFFFUL)
#define PORT_E_PIN_MASK_U32 ((uint32_t)0xFFFFFFFFUL)
// GPIO_PORT_F only has 9 pins of possible 32 - Mask with 0x1FF
#define PORT_F_PIN_MASK_U32 ((uint32_t)0x000001FFUL)


#define PORT_DRV_PORT_ID_FIRST ((Port_Drv_Id_t)(0U))

#define DEVICE_PART_NUMBER_ID_F28388D (0x03FF0300UL)
#define DEVICE_PART_NUMBER_ID_F28386D (0x03FD0300UL)
#define DEVICE_PART_NUMBER_ID_F28384D (0x03FB0300UL)
#define DEVICE_PART_NUMBER_ID_F28388S (0x03FF0400UL)
#define DEVICE_PART_NUMBER_ID_F28386S (0x03FD0400UL)
#define DEVICE_PART_NUMBER_ID_F28384S (0x03FB0400UL)

#define DEVICE_REVISION_ID_0 (0x0U)
#define DEVICE_REVISION_ID_A (0x1U)


// Package identifier for 176 pin package (extracted from Part Id)
#define DEVICE_PIN_COUNT_ID_176 (6)
// Package identifier for 337 pin package (extracted from Part Id)
#define DEVICE_PIN_COUNT_ID_337 (7)

// Maximum allowable GPIO pins in the largest package
// This is to stay consistent with the TI DriverLib
#define MAX_PORT_PINS (169U)


// Convert a pin 0-336 to a Port Id (A-F)
#define PIN_TO_PORT_ID(x) ((Port_Drv_Port_t)((x) >> 5U))

// Convert a pin 0-336 to a 0-31 pin value of a port (A-F)
#define PIN_TO_PORT_PIN(x) ((x) % NUM_PINS_PER_PORT)

// Valid Pin Index is 0-31
#define PORT_PIN_MASK_U16  ((uint16_t)0x001F)
// Convert pin index 0-31 to a bit mask in a single port (0x00000001..0x80000000)
#define PIN_INDEX_TO_PORT_MASK_U32(x)           ((uint32_t)((uint32_t)1<<PORT_PIN_MASK_U16(x)))

#define PIN_PACKAGE_ID_INVALID  (0U)

// GPIO Pin Mux byte 0 is 0 for GPIO, otherwise it is uses Alt. Function
// Ex. GPIO_31_GPIO31 (0x00081E00U) defined in pin_map.h
#define PORT_MODE_MASK (0x000000FFUL)

// Determine if the a GPIO mux configuration is GPIO or Alt. Function
#define IS_GPIO_MODE(x) ((PORT_MODE_GPIO == ((x) & PORT_MODE_MASK)))


/*******************************************************************************
// Private Type Declarations
*******************************************************************************/


typedef struct
{
    Port_Drv_PortId_t portId;
    GPIO_Port gpioPortId;
    uint32_t gpioPortMask;
} Port_Drv_t;

typedef enum
{
    PIN_PACKAGE_ID_176,
    PIN_PACKAGE_ID_337,
    PIN_PACKAGE_ID_COUNT
} PinPackageId_t;

typedef struct
{
    PinPackageId_t pinPackageId;
    uint16_t devicePackageId;
    uint16_t pinCount;
    const uint32_t *bondedPinMask;
} Package_Definition_t;

// Type used for port level values
// This is mask of bits 0-31 where '1' is bit set and '0' is bit cleared
 typedef uint32_t GPIO_PortLevel_t;



// This structure defines the internal variables used by the module
typedef struct
{
   // Configuration Table passed at Initialization
   Port_Drv_Config_t *portConfig;

   // Initialization state for the module
   bool isInitialized;

   // Enable state for the module
   bool enableState;

   uint16_t pinCount;
   PinPackageId_t packageIndex;

   // Pin mask for valid pins
   uint32_t validPinMask[PORT_DRV_PORT_ID_COUNT];
   uint32_t usedPinMask[PORT_DRV_PORT_ID_COUNT];
   uint32_t inputPinMask[PORT_DRV_PORT_ID_COUNT];
   uint32_t outputPinMask[PORT_DRV_PORT_ID_COUNT];
} Port_Drv_Status_t;


// Possible directions of a port pin
typedef enum
{
    // No settings: the pin is not available
    PORT_PIN_DIRECTION_DISABLED,  
    // Sets port pin as input
    PORT_PIN_DIRECTION_INPUT,            
    // Sets port pin as output
    PORT_PIN_DIRECTION_OUTPUT,            
    // Number of possible pin directions
    PORT_PIN_DIRECTION_COUNT            
} Port_PinDirection_t;

typedef enum
{
    INPUT_MODE_NONE,
    INPUT_MODE_FLOATING, //(Std)
    INPUT_MODE_PULLUP,
    INPUT_MODE_COUNT
} Input_Mode_t;

typedef enum
{
    OUTPUT_MODE_NONE,
    OUTPUT_MODE_PUSH_PULL, //(Std)
    OUTPUT_MODE_OPEN_DRAIN,
    OUTPUT_MODE_COUNT
} Output_Mode_t;

typedef struct
{
  Input_Mode_t inputMode;
  GPIO_QualificationMode inputQualificationMode;
  bool invert;
} InputConfig_t;

typedef uint32_t Pin_Level_t;
typedef struct
{
  Output_Mode_t outputMode;
  Pin_Level_t initialValue;
} OutputConfig_t;
   
// Mux Position:
// 0,4,8,12 are GPIO - (All are set)
// 1,2,3,5,6,7,15 are ALT FUNC
typedef enum
{
    PORT_MODE_GPIO,
    PORT_MODE_ALT_FUNC_1,
    PORT_MODE_ALT_FUNC_2,
    PORT_MODE_ALT_FUNC_3,
    PORT_MODE_ALT_FUNC_4_INVALID,
    PORT_MODE_ALT_FUNC_5,
    PORT_MODE_ALT_FUNC_6,
    PORT_MODE_ALT_FUNC_7,
    PORT_MODE_ALT_FUNC_8_INVALID,
    PORT_MODE_ALT_FUNC_9_INVALID,
    PORT_MODE_ALT_FUNC_10_INVALID,
    PORT_MODE_ALT_FUNC_11_INVALID,
    PORT_MODE_ALT_FUNC_12_INVALID,
    PORT_MODE_ALT_FUNC_13_INVALID,
    PORT_MODE_ALT_FUNC_14_INVALID,
    PORT_MODE_ALT_FUNC_15,
    PORT_MODE_COUNT,
} Port_PinMode_t;


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/


static const uint32_t bondedPinMask176[GPIO_PORT_COUNT_U16] =
{
// GPIO_PORT_A, 32 pins
(0xFFFFFFFFUL),
// GPIO_PORT_B, 32 pins
(0xFFFFFFFFUL),
// GPIO_PORT_C, 31 pins
(0x7FFFFFFFUL),
// GPIO_PORT_D, 1 pin
(0x00000008UL),
// GPIO_PORT_E, 1 pin
(0x00000020UL),
// GPIO_PORT_F, 0 pins
(0x00000000UL)
};

static const uint32_t bondedPinMask377[GPIO_PORT_COUNT_U16] =
{
    // GPIO_PORT_A, 32 pins
   (0xFFFFFFFFUL),
    // GPIO_PORT_B, 32 pins
   (0xFFFFFFFFUL),
    // GPIO_PORT_C, 32 pins
   (0xFFFFFFFFUL),
    // GPIO_PORT_D, 32 pins
   (0xFFFFFFFFUL),
    // GPIO_PORT_E, 32 pins
   (0xFFFFFFFFUL),
    // GPIO_PORT_F, 9 pins
   (0x000001FFUL)
};

// The variable used for holding all internal data for this module.
static Port_Drv_Status_t status;


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

static const Package_Definition_t packageDefinition[] =
{
    // 176-pin package port definitions
    {
        .pinPackageId = PIN_PACKAGE_ID_176,
        .devicePackageId = DEVICE_PIN_COUNT_ID_176,
        .pinCount = 176U,
        .bondedPinMask = bondedPinMask176
    },
    // 337-pin package port definitions
    {
        .pinPackageId = PIN_PACKAGE_ID_337,
        .devicePackageId = DEVICE_PIN_COUNT_ID_337,
        .pinCount = 337U,
        .bondedPinMask = bondedPinMask377
    }
};



static const Port_Drv_t portDefinition[PORT_DRV_PORT_ID_COUNT] =
{
    // Port A - 32 pins
    {
        .portId = PORT_DRV_PORT_ID_A,
        .gpioPortId = GPIO_PORT_A,
        .gpioPortMask = PORT_A_PIN_MASK_U32
    },
    // Port B - 32 pins
    {
        .portId = PORT_DRV_PORT_ID_B,
        .gpioPortId = GPIO_PORT_B,
        .gpioPortMask = PORT_B_PIN_MASK_U32
    },
    // Port C - 32 pins
    {
        .portId = PORT_DRV_PORT_ID_C,
        .gpioPortId = GPIO_PORT_C,
        .gpioPortMask = PORT_C_PIN_MASK_U32
    },
    // Port D - 32 pins
    {
        .portId = PORT_DRV_PORT_ID_D,
        .gpioPortId = GPIO_PORT_D,
        .gpioPortMask = PORT_D_PIN_MASK_U32
    },
    // Port E - 32 pins
    {
        .portId = PORT_DRV_PORT_ID_E,
        .gpioPortId = GPIO_PORT_E,
        .gpioPortMask = PORT_E_PIN_MASK_U32
    },
    // Port F - 9 pins
    {
        .portId = PORT_DRV_PORT_ID_F,
        .gpioPortId = GPIO_PORT_F,
        .gpioPortMask = PORT_F_PIN_MASK_U32
    },
};


/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

// Enable pullups on unbonded pins for the current package
static void InitUnbondedPins(PinPackageId_t packageIndex);


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/


static PinPackageId_t ConvertPackageIdToIndex(uint16_t devicePinCountId)
{

    //  Store package index - start with invalid index
    uint16_t packageIndex = PIN_PACKAGE_ID_INVALID;

    switch (devicePinCountId)
    {
    case DEVICE_PIN_COUNT_ID_176:
        //  Store package index
        packageIndex = PIN_PACKAGE_ID_176;
        break;
    case DEVICE_PIN_COUNT_ID_337:
        //  Store package index
        packageIndex = PIN_PACKAGE_ID_337;
        break;
    default:
        // Error - invalid package
        break;
    }

    // Return the converted package index
    return((PinPackageId_t)packageIndex);
}

// Function to enable pullups for the unbonded GPIOs on the package
static void InitUnbondedPins(PinPackageId_t packageIndex)
{
    // Verify the package index is valid
    if (packageIndex < PIN_PACKAGE_ID_COUNT)
    {
        // Disable pin locks for unbonded pins - ports A-F
        for (GPIO_Port gpioPortIndex = GPIO_PORT_A; gpioPortIndex <= GPIO_PORT_F; gpioPortIndex++)
        {
            // Verify that bonded pin index is valid
            if (gpioPortIndex < GPIO_PORT_COUNT_U16)
            {
                // See if the port has unbonded pins
                if (portDefinition[gpioPortIndex].gpioPortMask != packageDefinition[packageIndex].bondedPinMask[gpioPortIndex])
                {
                    // Port has unbonded pins to be configured
                    // Unlock all unbonded pins - make sure the pins are valid for the port (Ports A-E have 32 pins, Port E has 9 pins)
                    uint32_t pinMask = portDefinition[gpioPortIndex].gpioPortMask & ~packageDefinition[packageIndex].bondedPinMask[gpioPortIndex];
                    GPIO_unlockPortConfig(portDefinition[gpioPortIndex].gpioPortId, pinMask);
                }
            }
        }


        // Enable pullups for all unbonded pins in this package
        // Write 0 to enable pull-up for a pin (1 is pull-up disable)
        // First enable access to protected registers
        EALLOW;

        // Lop through each port and configure pullups on unbonded pins
        for (GPIO_Port gpioPortIndex = GPIO_PORT_A; gpioPortIndex <= GPIO_PORT_F; gpioPortIndex++)
        {
            // See if the port has unbonded pins
            if (portDefinition[gpioPortIndex].gpioPortMask != packageDefinition[packageIndex].bondedPinMask[gpioPortIndex])
            {
                // Calculate offset step for pullup configuration - 0xCU + 0x40U per port
                // 0x00007C00U + 0xCU + (PortIndex*0x40U)
                // Since pullup enable is 0, the mask will match the bonded pins
                // Ex. Port C (176-pin) pullup on pin 31 = 0x7FFFFFFFUL
                HWREG(GPIOCTRL_BASE + GPIO_O_GPAPUD + ((uint32_t)gpioPortIndex * GPIO_CTRL_REGS_STEP)) = packageDefinition[packageIndex].bondedPinMask[gpioPortIndex];
            }
        }

        // After writing, disable writes to protected registers
        EDIS;
    }
}

/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

bool Port_Drv_Init(const uint32_t moduleID, const Port_Drv_Config_t *configData)
{
    // Assume init failed until initialization is completed
    bool success = false;

    // Default module to uninitialized and not enabled
    status.isInitialized = false;
    status.enableState = false;

    // Assume no pins defined 
    status.pinCount = 0;

    // Get pin count for package - bits 8-10 have pin count identifier
    uint16_t pinCountId = SysCtl_getDeviceParametric(SYSCTL_DEVICE_PINCOUNT);

    // Convert the chip package identifier to a Package Enumeration
    status.packageIndex = ConvertPackageIdToIndex(pinCountId);

    // Verify the package index is valid
    if (status.packageIndex < PIN_PACKAGE_ID_COUNT) 
    {
        // Configure unbonded pins for the package package
        InitUnbondedPins(status.packageIndex);

        // Initialize the pin masks for each port
        for (GPIO_Port gpioPortIndex = GPIO_PORT_A; gpioPortIndex <= GPIO_PORT_F; gpioPortIndex++) 
        {
            // Package index is valid, initialize the valid pin mask for the port
            status.validPinMask[gpioPortIndex] = packageDefinition[status.packageIndex].bondedPinMask[gpioPortIndex];
            // Initialize to no configured pins for the port
            status.usedPinMask[gpioPortIndex] = 0U;
            status.inputPinMask[gpioPortIndex] = 0U;
            status.outputPinMask[gpioPortIndex] = 0U;
        }

        // Configure the pins specified by the board configuration
        // First, validate the given parameter is valid
        if (NULL != configData) 
        {
            // Store the given port configuration for later use
            status.portConfig = (Port_Drv_Config_t *)configData;

            // Loop through the port configuration and configure each pin
            for (uint32_t portPinIndex = 0; portPinIndex < status.portConfig->numConfigItems; portPinIndex++)
            {
                // Store the pin configuration structure for easy access
                Port_Drv_Data_t *pinConfig;
                pinConfig = (Port_Drv_Data_t *)(&(configData->pinConfigArray[portPinIndex]));

                uint32_t pinMask = 0;

                // Verify the pin is valid for the specified port and pin combination
                if (Port_Drv_IsPortPinValid((Port_Drv_PortId_t)pinConfig->devicePin.portId, pinConfig->devicePin.pinIndex))
                {
                    // Build mask for this pin
                    pinMask = PORT_PIN_TO_MASK(pinConfig->devicePin.pinIndex);

                    // Make sure the port is unlocked for this pin
                    GPIO_unlockPortConfig(pinConfig->devicePin.portId, pinMask);

                    // Port and pins are valid, see if pin is GPIO type or Alt. Func
                    if (IS_GPIO_MODE(pinConfig->pinMuxConfiguration))
                    {
                        // Pin is GPIO, check direction
                        if (GPIO_DIR_MODE_OUT == pinConfig->direction)
                        {
                            // GP Output
                            // Configure pin type using variation of TI driverlib function
                            GPIO_SetPortPinPadConfig(pinConfig->devicePin.portId, pinConfig->devicePin.pinIndex, pinConfig->gpioConfig.gpioOutput.outputPinType);

                            // Note that the pin has been configured for output before setting the initial value otherwise it will not be a valid write
                            status.outputPinMask[pinConfig->devicePin.portId] |= pinMask;

                            // Output, set the initial level before setting mode
                            Port_Drv_WritePortPin((Port_Drv_PortId_t)pinConfig->devicePin.portId, pinConfig->devicePin.pinIndex, GPIO_LEVEL_HIGH == pinConfig->gpioConfig.gpioOutput.initialLevel);

                            // Configure pin mux using TI driverlib
                            GPIO_setPinConfig(pinConfig->pinMuxConfiguration);

                            // Set output direction using variation of TI driverlib function
                            GPIO_SetPortPinDirectionMode(pinConfig->devicePin.portId, pinConfig->devicePin.pinIndex, pinConfig->direction);
                        }
                        else
                        {
                            // GP Input
                            // Configure pin type using using variation of TI driverlib function
                            GPIO_SetPortPinPadConfig(pinConfig->devicePin.portId, pinConfig->devicePin.pinIndex, pinConfig->gpioConfig.gpioInput.inputPinType);

                            // Configure pin mux using TI driverlib
                            //GPIO_setPinConfig(pinConfig->pinMuxConfiguration);

                            // Set input direction using variation of TI driverlib function
                            GPIO_SetPortPinDirectionMode(pinConfig->devicePin.portId, pinConfig->devicePin.pinIndex, pinConfig->direction);

                            // Note that the pin has been configured for output
                            status.inputPinMask[pinConfig->devicePin.portId] |= pinMask;
                        }

                        // Note that the pin is in use
                        status.usedPinMask[pinConfig->devicePin.portId] |= pinMask;
                    }
                    else
                    {
                        // Alt. Function - Check direction
                        if (GPIO_DIR_MODE_OUT == pinConfig->direction)
                        {
                            // AF Output
                            // Configure pin type using variation of TI driverlib function
                            GPIO_SetPortPinPadConfig(pinConfig->devicePin.portId, pinConfig->devicePin.pinIndex, pinConfig->gpioConfig.gpioOutput.outputPinType);

                            // Configure pin mux using TI driverlib
                            GPIO_setPinConfig(pinConfig->pinMuxConfiguration);
                        }
                        else
                        {
                            // AF Input
                            // Configure pin type using variation of TI driverlib function
                            GPIO_SetPortPinPadConfig(pinConfig->devicePin.portId, pinConfig->devicePin.pinIndex, pinConfig->gpioConfig.gpioInput.inputPinType);

                            // Set Async qualification mode for Alt. Funct peripheral
                            GPIO_SetPortPinQualificationMode(pinConfig->devicePin.portId, pinConfig->devicePin.pinIndex, pinConfig->gpioConfig.gpioInput.qualificationMode);

                            // Configure pin mux using TI driverlib
                            GPIO_setPinConfig(pinConfig->pinMuxConfiguration);
                        }

                        // Note that the pin is in use
                        status.usedPinMask[pinConfig->devicePin.portId] |= pinMask;
                    }

                } // Else pin Invalid
            } // End Pin Loop
        }

        // Init complete
        success = true;
    }
    else
    {
        // Error - Unknown package ID
        ASSERT(status.packageIndex < PIN_PACKAGE_ID_COUNT);
    }

    // Return the result
    return(success);
}

bool Port_Drv_IsPortPinValid(Port_Drv_PortId_t portId, Port_Drv_Port_Pin_t portPinIndex)
{
    // Verify port index is valid 0..6 (PortA..Port F)
    // If valid, mask with the valid pin mask for the port
    return((portId < PORT_DRV_PORT_ID_COUNT) && (status.validPinMask[portId] & PORT_PIN_TO_MASK(portPinIndex)));
}




Port_Drv_Mask_t Port_Drv_ReadMaskedPort(Port_Drv_PortId_t portId, Port_Drv_Level_t dataMask)
{
    // Invalid/unconfigured pins return low level
    uint32_t portData = 0UL;

    // Validate the given port
    if (portId < PORT_DRV_PORT_ID_COUNT)
    {
        // Port Id is valid, convert logical Port ID to TI GPIO Port
        GPIO_Port gpioPort = portDefinition[portId].gpioPortId;

        // Determine the mask for configured input and output pins -- either can be read
        // The validOuputPinMask will handle any invalid mask
        uint32_t validatedMask = dataMask & (status.inputPinMask[portId] | status.outputPinMask[portId]);

        // --- Read entire port using using TI DriverLib function
        // Any unconfigured pins will be masked out
        portData = GPIO_readPortData(gpioPort) & validatedMask;
    }

    // Return the read data
    return(portData);
}


// This will set/clear the pins in the given mask port a port
// Pins not configured for output will not be changed
void Port_Drv_WriteMaskedPort(Port_Drv_PortId_t portId, Port_Drv_Level_t dataMask, uint32_t portData)
{
    // Validate the given port
    if (portId < PORT_DRV_PORT_ID_COUNT)
    {
        // Port Id is valid, convert logical Port ID to TI GPIO Port
        GPIO_Port gpioPort = portDefinition[portId].gpioPortId;

        // Determine the mask for configured output pins -- this is used for both set and clear
        // The validOuputPinMask will handle any invalid mask
        // If pin has not been configured as output, nothing will be changed
        uint32_t validatedMask = dataMask & status.outputPinMask[portId];

        // --- Set all configured bits using TI DriverLib function
        // If the masked data is 0, nothing will be changed
        GPIO_setPortPins(gpioPort, portData & validatedMask);

        // --- Clear all configured bits using TI DriverLib function
        // If the masked data is 0, nothing will be changed
        GPIO_clearPortPins(gpioPort, (~portData) & validatedMask);
    }
}

// This will toggle the pins in the given mask port a port
// Pins not configured for output will not be changed
Port_Drv_Mask_t Port_Drv_ToggleMaskedPort(Port_Drv_PortId_t portId, Port_Drv_Level_t dataMask)
{
    uint32_t channelLevel = 0U;

    // Validate the given port
    if (portId < PORT_DRV_PORT_ID_COUNT)
    {
        // Port Id is valid, convert logical Port ID to TI GPIO Port
        GPIO_Port gpioPort = portDefinition[portId].gpioPortId;

        // Determine the mask for configured output pins -- this is used for both set and clear
        // The validOuputPinMask will handle any invalid mask
        // If pin has not been configured as output, nothing will be changed
        uint32_t validatedMask = dataMask & status.outputPinMask[portId];

        // --- Toggle all configured bits using TI DriverLib function
        // If the masked data is 0, nothing will be changed
        GPIO_togglePortPins(gpioPort, validatedMask);

        // Read back the pin for the return value
        // Low is 0, High otherwise
        channelLevel = GPIO_readPortData(gpioPort) & validatedMask;
    }

    // Return the level for the port
    return ((Port_Drv_Mask_t)channelLevel);
}


// This will read all valid pins for the port
// Pins not configured will read back as 0
Port_Drv_Level_t Port_Drv_ReadPort(Port_Drv_PortId_t portId)
{
    // Read the entire port using the maximum 32 pin mask
    // Any invalid pins will be handled by the masked read function
    return(Port_Drv_ReadMaskedPort(portId, PORT_PIN_MASK_U32));
}

// This will set/clear all valid pins for the port
// Pins not configured for output will not be changed
void Port_Drv_WritePort(Port_Drv_PortId_t portId, Port_Drv_Level_t portData)
{
    // Write the entire port using the maximum 32 pin mask
    // Any invalid pins will be handled by the masked write function
    Port_Drv_WriteMaskedPort(portId, PORT_PIN_MASK_U32, portData);
}

// This will Toggle all valid pins for the port
// Pins not configured for output will not be changed
Port_Drv_Mask_t Port_Drv_TogglePort(Port_Drv_PortId_t portId)
{
    // Call toggle function with the entire port using the maximum 32 pin mask
    // Any invalid pins will be handled by the masked toggle functions
    return(Port_Drv_ToggleMaskedPort(portId, PORT_PIN_MASK_U32));
}



bool Port_Drv_ReadPortPin(Port_Drv_PortId_t portId, Port_Drv_Port_Pin_t portPin)
{
    // Init to low value for invalid or unconfigured pin
    bool activeState = false;

    // Build initial mask that is to be passed to Set/Clear register
    // Mask Write function will handle validating the pin
    uint32_t pinMask = (uint32_t)(1UL << portPin);

    // Read pin value using mask - low will read 0 at pin position
    // Mask will prevent invalid pins from being read
    activeState = (pinMask == Port_Drv_ReadMaskedPort(portId, pinMask));

    // Return the state
    return(activeState);
}

void Port_Drv_WritePortPin(Port_Drv_PortId_t portId, Port_Drv_Port_Pin_t portPin, bool activeState)
{
    // Build initial mask that is to be passed to Set/Clear register
    // Mask Write function will handle validating the pin
    uint32_t pinMask = (uint32_t)(1UL << portPin);

    // Check if we are setting or clearing the pin
    if (activeState)
    {
        // Data to be written is just the same as the mask
        // Mask will prevent any other pins from being set
        Port_Drv_WriteMaskedPort(portId, pinMask, pinMask);
    }
    else
    {
        // Just use 0 as the low data
        // Mask will prevent any other pins from being cleared
        Port_Drv_WriteMaskedPort(portId, pinMask, 0);
    }
}


bool Port_Drv_TogglePortPin(Port_Drv_PortId_t portId, Port_Drv_Port_Pin_t portPin)
{
    // Init to low value for invalid or unconfigured pin
    bool activeState = false;

    // Build initial mask that is to be passed to Set/Clear register
    // Mask Write function will handle validating the pin
    uint32_t pinMask = (uint32_t)(1UL << portPin);

    // Toggle pin value using mask - low will read 0 at pin position
    // Mask will prevent invalid pins from being read
    activeState = (0 != Port_Drv_ToggleMaskedPort(portId, pinMask));

    // Return the state
    return(activeState);
}
