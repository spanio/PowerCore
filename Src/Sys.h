/*******************************************************************************
// System Module 
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once


/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Sys_Config.h"
// Platform Includes
#include "MessageRouter.h"
// Other Includes
#include <stdint.h> // Defines C99 integer types


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

// Macro to call SysCtl_delay() to achieve a delay in microseconds. The macro
// will convert the desired delay in microseconds to the count value expected
// by the function. \b x is the number of microseconds to delay.
#define SYS_DELAY_US(x) SysCtl_delay(((((long double)(x)) / (1000000.0L /  \
                              (long double)SYS_SYSCLK_FREQ)) - 9.0L) / 5.0L)

/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

// Defines the type used for returning system configuration information
typedef Sys_Product_Config_t *const Sys_ProductConfigPtr_t;

// Common configuration structure passed to the module initialization function
// Data is generally defined in the board-specific configuration file
typedef struct
{
    // The number of items in the sysConfigData - calculated by compiler
    uint32_t numConfigItems;
    // Configuration data
    const struct Sys_Data_s *dataPtr;
} Sys_Config_t;


/*******************************************************************************
// Public Variable Definition
*******************************************************************************/

// Chip Identifier located on startup
// Eg. TM320F2879D us is 0x**F90300
extern uint16_t Sys_Drv_chipId[SYS_NUM_CHIP_ID_BYTES/2U];

// Defines, Globals, and Header Includes related to Flash Support
#ifdef _FLASH

extern uint16_t RamfuncsLoadStart;
extern uint16_t RamfuncsLoadEnd;
extern uint16_t RamfuncsLoadSize;
extern uint16_t RamfuncsRunStart;
extern uint16_t RamfuncsRunEnd;
extern uint16_t RamfuncsRunSize;

#define DEVICE_FLASH_WAITSTATES 3

#endif

// Check to see if the CPU is the master (CPU ID 1)
#define SYS_IS_CPU_ID_MASTER(x) (SYS_CPU_ID_1 == (x))


/*******************************************************************************
// Public Function Declarations
*******************************************************************************/

/*******************************************************************************
// Description:
//    Get the logicial identifier of the current CPU.
// Parameters:
//    none
// Returns: 
//    SYS_CPU_ID_1 - CPU 1 (Master/Default)
//    SYS_CPU_ID_2 - CPU 2
*******************************************************************************/
Sys_CpuId_t Sys_GetCpuId(void);


/*******************************************************************************
// Description:
//    Intialized the system clocks and PLL. 
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
bool Sys_Init(const uint32_t moduleId, const Sys_Config_t *configData);

/*******************************************************************************
// Description:
//    Return the system configuration information.  This is primarily used
//    to get the system Version and Product identifiers.
// Parameters:
//    none
// Returns: 
//    Sys_ConfigPtr_t - Pointer to the static configuration structure.
*******************************************************************************/
Sys_Product_Config_Ptr_t Sys_GetSystemInfo(void);

Sys_Version_t Sys_GetSystemVersion(void);

uint32_t Sys_GetSystemNumericVersion(void);

Sys_ReleaseConfiguration_t Sys_GetReleaseConfiguration(void);

Sys_ProductId_t Sys_GetProductId(void);

Sys_ProductName_t Sys_GetProductName(void);

uint32_t Sys_GetClockFrequencyHz(void);
    
uint32_t Sys_LowSpeedClockFrequencyHz(void);

void Sys_MessageRouter_GetApplicationVersion(MessageRouter_Message_t *const message);
void Sys_MessageRouter_GetProductID(MessageRouter_Message_t *const message);
void Sys_MessageRouter_GetProductName(MessageRouter_Message_t *const message);
void Sys_MessageRouter_GetResetReason(MessageRouter_Message_t *const message);
void Sys_MessageRouter_Reset(MessageRouter_Message_t *const message);
void Sys_MessageRouter_GetUptimeMillseconds(MessageRouter_Message_t *const message);

/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif

