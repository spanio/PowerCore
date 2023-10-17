/*******************************************************************************
// System Driver Configuration
*******************************************************************************/
// Prevent multiple inclusion of header file
#pragma once


/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
// Platform Includes
// Other Includes
#include "sysctl.h" // TI System Control Library
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


// ------------------------------------
// SYSTEM CLOCK SOURCE
// ------------------------------------

// Select one of three possible clock sources
// Choose Internal Backup Oscillator (INTOSC1) as PLL Clock Source
//#define PLL_CLOCK_SRC SYSCTL_OSCSRC_OSC1
// Choose Internal Primary Oscillator (INTOSC2) as PLL Clock Source
//#define PLL_CLOCK_SRC SYSCTL_OSCSRC_OSC2
// Choose External Crystal as PLL Clock Source
#define PLL_CLOCK_SRC SYSCTL_OSCSRC_XTAL

// Define XTAL as 20MHz for this board configruation
#if PLL_CLOCK_SRC == SYSCTL_OSCSRC_XTAL

// 25MHz XTAL on controlCARD. For use with SysCtl_getClock() and
// SysCtl_getAuxClock().
#define SYS_OSCSRC_FREQ          25000000U

// Internal Osc. is 10MHz
#elif (PLL_CLOCK_SRC == SYSCTL_OSCSRC_OSC1) ||  (PLL_CLOCK_SRC == SYSCTL_OSCSRC_OSC2)
// Default internal oscillator frequency, 10 MHz (Defined in sysctl.h)
#define SYS_OSCSRC_FREQ SYSCTL_DEFAULT_OSC_FREQ

#else
#error "Invalid PLL Clock Source"

#endif // PLL_CLOCK_SRC


// ------------------------------------
// SYSTEM CLOCK CONFIGURATION
// ------------------------------------

// PLL multipler = 20.0
#define PLL_INTEGER_MULTIPLIER (20)
// Currently not using fractional multipliers - set as 1 and use SYSCTL_FMULT_NONE
#define PLL_FRACTIONAL_MULTIPLIER (1)
// SysClk divider 2 will equal allow max. frequency of 200MHz
#define PLL_SYSCLK_DIVIDER (2)

// Define to pass to SysCtl_setClock(). Will configure the clock as follows:
// PLLSYSCLK = 20MHz (XTAL_OSC) * 20 (IMULT) * 1 (FMULT) / 2 (PLLCLK_BY_2)
#define SYS_SETCLOCK_CFG         (PLL_CLOCK_SRC | SYSCTL_IMULT(PLL_INTEGER_MULTIPLIER) |  \
                                  SYSCTL_FMULT_NONE | SYSCTL_SYSDIV(PLL_SYSCLK_DIVIDER) |   \
                                  SYSCTL_PLL_ENABLE)

/*
// Define to pass to SysCtl_setClock(). Will configure the clock as follows:
// PLLSYSCLK = 20MHz (XTAL_OSC) * 20 (IMULT) * 1 (FMULT) / 2 (PLLCLK_BY_2)
//
#define DEVICE_SETCLOCK_CFG         (SYSCTL_OSCSRC_XTAL | SYSCTL_IMULT(20) |  \
                                     SYSCTL_FMULT_NONE | SYSCTL_SYSDIV(2) |   \
                                     SYSCTL_PLL_ENABLE)
*/

// 200MHz SYSCLK frequency computed based on the above SYS_SETCLOCK_CFG
#define SYS_SYSCLK_FREQ          ((SYS_OSCSRC_FREQ * PLL_INTEGER_MULTIPLIER * PLL_FRACTIONAL_MULTIPLIER) / PLL_SYSCLK_DIVIDER)


// ------------------------------------
// LOW SPEED CLOCK
// ------------------------------------

// Low speed clock prescaler (Must be: 1, 2, 4, 6, 8, 10, 12, or 14)
// Default is divide by 4
#define SYS_LSPCLK_DIVIDER (4)

// Determine Presecale value used by SysCtl_setLowSpeedClock
#if (SYS_LSPCLK_DIVIDER == 1)
// Prescale value of 1 is setting 0
#define SYS_LSPCLK_PRESCALE   SYSCTL_LSPCLK_PRESCALE_1

// Prescaler of 2-14 is divided by 2
#elif ((SYS_LSPCLK_DIVIDER >= 2) && (SYS_LSPCLK_DIVIDER <= 14))
// Prescaler of 1-14 is divided by 2
#define SYS_LSPCLK_PRESCALE   (SysCtl_LSPCLKPrescaler)(SYS_LSPCLK_DIVIDER >> 1U)

#else
#error "Invalid Low Speed Divider"

#endif // SYS_LSPCLK_DIVIDER


// 50MHz LSPCLK frequency based on the above DEVICE_SYSCLK_FREQ and a default
// low speed peripheral clock divider of 4. Update the code below if a
// different LSPCLK divider is used!
#define SYS_LSPCLK_FREQ          (SYS_SYSCLK_FREQ / SYS_LSPCLK_DIVIDER)


// ------------------------------------
// FLASH ACCESS
// ------------------------------------

// Define the number of wait-states for a flash access
// (See Flash_setWaitstates() for more details)
#define SYS_FLASH_WAITSTATES 3

// Chip identifier length (2 x 16-bit values)
#define SYS_NUM_CHIP_ID_BYTES (4U)

#define SYS_PRODUCT_NAME_LENGTH (20U)



/*******************************************************************************
// Public Type Declarations
*******************************************************************************/

// Semantic version information for the system in the format Major.Minor.Build
// Note that the sizes are defined such that a complete version can be grouped into
// an unsigned 32-bit value used for machine comparison of versions.
// this will also protect against byte alignment issues.
// Examples: Version 1.2.3 (0x01020003) is greater than Version 1.1.4 (0x01010004)
// Version 2.x.x denotes that some parts of the system may not be compatible with Version 1.x.x
// Version 1.2.x is expected to be compatible with Version 1.1.x
// Version 1.1.2 identifies firmware that differs from (but is compatible with) Version 1.1.1
typedef struct
{
   // Defines the Major component of a semantic version (Major.Minor.Build)
   uint16_t major;
   // Defines the Minor component of a semantic version (Major.Minor.Build)
   uint16_t minor;
   // Defines the Build component of a semantic version (Major.Minor.Build)
   uint16_t build;
} Sys_Version_t;

// Defines the type used for identifying the configuration of a release
// Explicit definition of the this value helps identify when a non-release
// compilation was used.
typedef enum
{
   SYS_RELEASE_CONFIGURATION_DEBUG,
   SYS_RELEASE_CONFIGURATION_RELEASE,
   SYS_RELEASE_CONFIGURATION_COUNT
} Sys_ReleaseConfiguration_t;

// Defines the type used for identifying a machine-readable product identifier
typedef uint16_t Sys_ProductId_t;

// Defines the type used for string containing an the human-readable product name
// in ASCII characters
// Note that the string is placed into a structure so that Sys_ProductName_t can
// be passed by reference but still allows sizeof(productName->stringValue) to
// return the correct length of the string rather than the size of the pointer
// to the string.
typedef struct
{
    /** This string may be used to provide additional details about
  * the firmware that the version number can't provide. This is
  * intended to provide an easier string identification method
  * for users. This is placed in a structure to so that
  * Sys_ProductName_t can be passed by reference and the
  * user can use sizeof(productName-\>stringValue) to return the
  * proper array size rather than pointer size.
  * Note:
  *    Make sure that the data buffer used for the Message Router
  *    response is large enough to hold the string you choose.
  */
    char stringValue[SYS_PRODUCT_NAME_LENGTH];
} Sys_ProductName_t;

typedef struct
{
   // Numeric Identifiers for Product Version
   Sys_Version_t version;
   // Numeric Identifier for Release Configuration
   Sys_ReleaseConfiguration_t releaseConfiguration;
   // Numeric Identifier for Product
   Sys_ProductId_t productId;
   // String Identifier for Product
   // Note: this is placed last to prevent byte alignment problems
   Sys_ProductName_t productName;
} Sys_Product_Config_t;


typedef enum {
   SYS_CPU_ID_1,
   SYS_CPU_ID_2,
   SYS_CPU_ID_COUNT
} Sys_CpuId_t;

typedef struct Sys_Product_Config_t *Sys_Product_Config_Ptr_t;
typedef struct {

    Sys_CpuId_t cpuId;
    Sys_Product_Config_t productConfig;
} Sys_Channel_Config_t;


/*******************************************************************************
// Public Variable Definitions
*******************************************************************************/


/*******************************************************************************
// End of C Binding Section
*******************************************************************************/
#ifdef __cplusplus
}
#endif

