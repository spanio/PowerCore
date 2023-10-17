/*******************************************************************************
// System Module - TI F2838x Implementation
*******************************************************************************/

/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "Sys.h"
#include "Sys_Config.h"
#include "Sys_ConfigTypes.h"
// Platform Includes
#include "MessageRouter.h"
#include "Reset_Drv.h"
#include "Timebase.h"
// Other Includes
#include "F2838x_device.h"
#include "driverlib.h" // TI DriverLib Definitions
#include <limits.h> // CHAR_BIT definition (for 8/16-bit chars)
#include <stddef.h> // size_t
#include <time.h>
// IPC included only on F28388D device
#if (F2838xD == TARGET)
#include "inc/hw_ipc.h"
#endif

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

// The macros that can be used as parameter to the function Device_bootCPU2
#define C1C2_BROM_BOOTMODE_BOOT_FROM_PARALLEL                        0x00000000U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_SCI                             0x00000001U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_SPI                             0x00000004U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_I2C                             0x00000005U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_CAN                             0x00000007U
#define C1C2_BROM_BOOTMODE_BOOT_FROM_RAM                             0x0000000AU
#define C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH                           0x0000000BU

// Other macros that are needed for the Device_bootCPU2 function
#define BROM_IPC_EXECUTE_BOOTMODE_CMD                                0x00000013U
#define C1C2_BROM_BOOTMODE_BOOT_COMMAND_MAX_SUPPORT_VALUE            0x0000000CU
#define C2_BOOTROM_BOOTSTS_C2TOC1_IGNORE                             0x00000000U
#define C2_BOOTROM_BOOTSTS_SYSTEM_START_BOOT                         0x00000001U
#define C2_BOOTROM_BOOTSTS_SYSTEM_READY                              0x00000002U
#define C2_BOOTROM_BOOTSTS_C2TOC1_BOOT_CMD_ACK                       0x00000003U
#define C2_BOOTROM_BOOTSTS_C2TOC1_BOOT_CMD_NAK_STATUS_NOT_SUPPORTED  0x00000004U
#define C2_BOOTROM_BOOTSTS_C2TOC1_BOOT_CMD_NAK_STATUS_BUSY_WITH_BOOT 0x00000005U

/*******************************************************************************
// Private Type Declarations
*******************************************************************************/

// This structure defines the internal variables used by the module
typedef struct
{
    // Module Id given to this module at Initialization
    uint16_t moduleId;

    // Configuration Table passed at Initialization
    const Sys_Config_t *sysConfig;

    // Initialization state for the module
    bool isInitialized;

    // Holds the chip identifier for the system
    uint16_t Sys_Drv_chipId[SYS_NUM_CHIP_ID_BYTES >> 1U];

} Sys_Status_t;


/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/

// The variable used for holding all internal data for this module.
static Sys_Status_t status;



/*******************************************************************************
// Private Function Declarations
*******************************************************************************/

void ReadChipInformation(void);

//*****************************************************************************
//! Executes a CPU02 control system bootloader.
//!
//! \param bootMode specifies which CPU02 control system boot mode to execute.
//!
//! This function will allow the CPU01 master system to boot the CPU02 control
//! system via the following modes: Boot to RAM, Boot to Flash, Boot via SPI,
//! SCI, I2C, or parallel I/O. This function blocks and waits until the
//! control system boot ROM is configured and ready to receive CPU01 to CPU02
//! IPC INT0 interrupts. It then blocks and waits until IPC INT0 and
//! IPC FLAG31 are available in the CPU02 boot ROM prior to sending the
//! command to execute the selected bootloader.
//!
//! The \e bootMode parameter accepts one of the following values:
//!  - \b C1C2_BROM_BOOTMODE_BOOT_FROM_PARALLEL
//!  - \b C1C2_BROM_BOOTMODE_BOOT_FROM_SCI
//!  - \b C1C2_BROM_BOOTMODE_BOOT_FROM_SPI
//!  - \b C1C2_BROM_BOOTMODE_BOOT_FROM_I2C
//!  - \b C1C2_BROM_BOOTMODE_BOOT_FROM_CAN
//!  - \b C1C2_BROM_BOOTMODE_BOOT_FROM_RAM
//!  - \b C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH
//!
//! \return true0 (success) if command is sent, or false (failure) if boot mode is
//! invalid and command was not sent.
//
//*****************************************************************************
bool BootCPU2(uint32_t bootMode);

void ConfigureTMXAnalogTrim(void);


/*******************************************************************************
// Private Function Implementations
*******************************************************************************/

void ReadChipInformation(void)
{
    // PART ID only valid for CPU 1
#ifdef CPU1
    // Use driverlib to get Part ID - TMS320F28388D is 0x**F90300
    status.Sys_Drv_chipId[0] = SysCtl_getDeviceParametric(SYSCTL_DEVICE_FAMILY);
    status.Sys_Drv_chipId[1] = SysCtl_getDeviceParametric(SYSCTL_DEVICE_PARTNO);
    // TODO - Get unique ID for this chip
    //Sys_Drv_chipSerial = ;
#endif
}

//*****************************************************************************
//
// Function to verify the XTAL frequency
// freq is the XTAL frequency in MHz
// The function return true if the the actual XTAL frequency matches with the
// input value
//
// Note that this function assumes that the PLL is not already configured and
// hence uses SysClk freq = 10MHz for DCC calculation
//
//*****************************************************************************
#ifdef CPU1
bool Sys_VerifyXTAL(float freq)
{
    //
    // Use DCC to verify the XTAL frequency using INTOSC2 as reference clock
    //

    //
    // Enable DCC0 clock
    //
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_DCC0);

    //
    // Insert atleast 5 cycles delay after enabling the peripheral clock
    //
    asm(" RPT #5 || NOP");

    //
    // Configures XTAL as CLKSRC0 and INTOSC2 as CLKSRC1
    // Fclk0 = XTAL frequency (input parameter)
    // Fclk1 = INTOSC2 frequency = 10MHz
    //
    // Configuring DCC error tolerance of +/-1%
    // INTOSC2 can have a variance in frequency of +/-10%
    //
    // Assuming PLL is not already configured, SysClk freq = 10MHz
    //
    // Note : Update the tolerance and INTOSC2 frequency variance as necessary.
    //
    return (DCC_verifyClockFrequency(DCC0_BASE,
                                     DCC_COUNT1SRC_INTOSC2, 10.0F,
                                     DCC_COUNT0SRC_XTAL, freq,
                                     1.0F, 10.0F, 10.0F));

}
#endif

#ifdef CPU1

// Only include dual boot in dual-core version
#if (F2838xD == TARGET)
bool BootCPU2(uint32_t bootMode)
{
    uint32_t bootStatus;
    uint16_t pin;
    bool returnVal = true;

    //
    // If CPU2 has already booted, return a fail to let the application
    // know that something is out of the ordinary.
    //
    bootStatus = HWREG(IPC_BASE + IPC_O_BOOTSTS) & 0x0000000FU;

    if(bootStatus == C2_BOOTROM_BOOTSTS_C2TOC1_BOOT_CMD_ACK)
    {
        //
        // Check if MSB is set as well
        //
        bootStatus = ((uint32_t)(HWREG(IPC_BASE + IPC_O_BOOTSTS) &
                                 0x80000000U)) >> 31U;

        if(bootStatus != 0)
        {
            returnVal = false;

            return returnVal;
        }
    }

    //
    // Wait until CPU02 control system boot ROM is ready to receive
    // CPU01 to CPU02 INT1 interrupts.
    //
    do
    {
        bootStatus = HWREG(IPC_BASE + IPC_O_BOOTSTS) &
                     C2_BOOTROM_BOOTSTS_SYSTEM_READY;
    } while ((bootStatus != C2_BOOTROM_BOOTSTS_SYSTEM_READY));

    //
    // Loop until CPU02 control system IPC flags 1 and 32 are available
    //
    while (((HWREG(IPC_BASE + IPC_O_FLG) & IPC_FLG_IPC0)  != 0U) ||
           ((HWREG(IPC_BASE + IPC_O_FLG) & IPC_FLG_IPC31) != 0U))
    {

    }

    if (bootMode >= C1C2_BROM_BOOTMODE_BOOT_COMMAND_MAX_SUPPORT_VALUE)
    {
        returnVal = false;
    }
    else
    {
        //
        // Based on boot mode, enable pull-ups on peripheral pins and
        // give GPIO pin control to CPU02 control system.
        //
        switch (bootMode)
        {
            case C1C2_BROM_BOOTMODE_BOOT_FROM_SCI:

                 //
                 //SCIA connected to CPU02
                 //
                 SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL5_SCI, 1,
                                               SYSCTL_CPUSEL_CPU2);

                 //
                 //Allows CPU02 bootrom to take control of clock
                 //configuration registers
                 //
                 EALLOW;
                 HWREG(CLKCFG_BASE + SYSCTL_O_CLKSEM) = 0xA5A50000U;
                 HWREG(CLKCFG_BASE + SYSCTL_O_LOSPCP) = 0x0002U;
                 EDIS;

                 GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
                 GPIO_setQualificationMode(29, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_29_SCITXDA);
                 GPIO_setMasterCore(29, GPIO_CORE_CPU2);

                 GPIO_setDirectionMode(28, GPIO_DIR_MODE_IN);
                 GPIO_setQualificationMode(28, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_28_SCIRXDA);
                 GPIO_setMasterCore(28, GPIO_CORE_CPU2);

                break;

            case C1C2_BROM_BOOTMODE_BOOT_FROM_SPI:

                 //
                 //SPI-A connected to CPU02
                 //
                 SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL6_SPI, 1,
                                               SYSCTL_CPUSEL_CPU2);

                 //
                 //Allows CPU02 bootrom to take control of clock configuration
                 // registers
                 //
                 EALLOW;
                 HWREG(CLKCFG_BASE + SYSCTL_O_CLKSEM) = 0xA5A50000U;
                 EDIS;

                 GPIO_setDirectionMode(16, GPIO_DIR_MODE_IN);
                 GPIO_setQualificationMode(16, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_16_SPISIMOA);
                 GPIO_setMasterCore(16, GPIO_CORE_CPU2);

                 GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
                 GPIO_setQualificationMode(17, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_17_SPISOMIA);
                 GPIO_setMasterCore(17, GPIO_CORE_CPU2);

                 GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
                 GPIO_setQualificationMode(18, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_18_SPICLKA);
                 GPIO_setMasterCore(18, GPIO_CORE_CPU2);

                 GPIO_setDirectionMode(19, GPIO_DIR_MODE_OUT);
                 GPIO_setQualificationMode(19, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_19_GPIO19);
                 GPIO_setMasterCore(19, GPIO_CORE_CPU2);

                break;

            case C1C2_BROM_BOOTMODE_BOOT_FROM_I2C:

                 //
                 //I2CA connected to CPU02
                 //
                 SysCtl_selectCPUForPeripheral(SYSCTL_CPUSEL7_I2C, 1,
                                               SYSCTL_CPUSEL_CPU2);

                 //
                 //Allows CPU2 bootrom to take control of clock
                 //configuration registers
                 //
                 EALLOW;
                 HWREG(CLKCFG_BASE + SYSCTL_O_CLKSEM) = 0xA5A50000U;
                 HWREG(CLKCFG_BASE + SYSCTL_O_LOSPCP) = 0x0002U;
                 EDIS;

                 GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
                 GPIO_setQualificationMode(32, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_32_SDAA);
                 GPIO_setMasterCore(32, GPIO_CORE_CPU2);

                 GPIO_setDirectionMode(33, GPIO_DIR_MODE_IN);
                 GPIO_setQualificationMode(33, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_33_SCLA);
                 GPIO_setMasterCore(33, GPIO_CORE_CPU2);

                break;

            case C1C2_BROM_BOOTMODE_BOOT_FROM_PARALLEL:

                 for(pin=58;pin<=65;pin++)
                 {
                     GPIO_setDirectionMode(pin, GPIO_DIR_MODE_IN);
                     GPIO_setQualificationMode(pin, GPIO_QUAL_ASYNC);
                     GPIO_setMasterCore(pin, GPIO_CORE_CPU2);
                 }

                 GPIO_setDirectionMode(69, GPIO_DIR_MODE_IN);
                 GPIO_setQualificationMode(69, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_69_GPIO69);
                 GPIO_setMasterCore(69, GPIO_CORE_CPU2);

                 GPIO_setDirectionMode(70, GPIO_DIR_MODE_IN);
                 GPIO_setQualificationMode(70, GPIO_QUAL_ASYNC);
                 GPIO_setPinConfig(GPIO_70_GPIO70);
                 GPIO_setMasterCore(70, GPIO_CORE_CPU2);

                 break;


            case C1C2_BROM_BOOTMODE_BOOT_FROM_CAN:
                 //
                 //Set up the GPIO mux to bring out CANATX on GPIO71
                 //and CANARX on GPIO70
                 //
                 GPIO_unlockPortConfig(GPIO_PORT_C, 0xFFFFFFFFU);

                 GPIO_setMasterCore(71, GPIO_CORE_CPU2);
                 GPIO_setPinConfig(GPIO_71_CANTXA);
                 GPIO_setQualificationMode(71, GPIO_QUAL_ASYNC);

                 GPIO_setMasterCore(70, GPIO_CORE_CPU2);
                 GPIO_setPinConfig(GPIO_70_CANRXA);
                 GPIO_setQualificationMode(70, GPIO_QUAL_ASYNC);


                 GPIO_lockPortConfig(GPIO_PORT_C, 0xFFFFFFFFU);

                 //
                 // Set CANA Bit-Clock Source Select = SYSCLK and enable CAN
                 //
                 EALLOW;
                 HWREG(CLKCFG_BASE + SYSCTL_O_CLKSRCCTL2) &=
                                            SYSCTL_CLKSRCCTL2_CANABCLKSEL_M;
                 EDIS;
                 SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_CANA);

               break;

         }

        //
        //CPU01 to CPU02 IPC Boot Mode Register
        //
        HWREG(IPC_BASE + IPC_O_BOOTMODE) = bootMode;

        //
        // CPU01 To CPU02 IPC Command Register
        //
        HWREG(IPC_BASE + IPC_O_SENDCOM) = BROM_IPC_EXECUTE_BOOTMODE_CMD;

        //
        // CPU01 to CPU02 IPC flag register
        //
        HWREG(IPC_BASE + IPC_O_SET) = 0x80000001U;

    }
    return returnVal;
}
#endif // F2838xD




/*******************************************************************************
// Public Function Implementations
*******************************************************************************/

// Fetch the current CPU identifier
Sys_CpuId_t Sys_GetCpuId(void)
{
// User compilation flag to determine CPU
#ifdef CPU2
   // If CPU2 flag defined, this is CPU ID 2
   return(SYS_CPU_ID_2);
#else
   // Otherwise, default to CPU ID 1
   return (SYS_CPU_ID_1);
#endif
}

// Initialize the CPU clock
bool Sys_Init(const uint32_t moduleId, const Sys_Config_t *configData)
{
    // Disable the watchdog
    SysCtl_disableWatchdog();

#ifdef _FLASH
    //
    // Copy time critical code and flash setup code to RAM. This includes the
    // following functions: InitFlash();
    //
    // The RamfuncsLoadStart, RamfuncsLoadSize, and RamfuncsRunStart symbols
    // are created by the linker. Refer to the device .cmd file.
    //
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);

    //
    // Call Flash Initialization to setup flash waitstates. This function must
    // reside in RAM.
    //
    Flash_initModule(FLASH0CTRL_BASE, FLASH0ECC_BASE, SYS_FLASH_WAITSTATES);
#endif

    // Default module to uninitialized and not enabled
    status.isInitialized = false;
    // Store the module Id for error reporting
    status.moduleId = moduleId;

    // First, validate the given parameter is valid
    if ((NULL != configData) && (NULL != configData->dataPtr))
    {
        // Store the given configuration table
        status.sysConfig = (Sys_Config_t *)configData;

        // Get device parametric configuration to read CPU ID
        ReadChipInformation();

        // Only configure clock if this is CPU1
        if (Device_isCPU1())
        {
            //
            // Verify the crystal frequency.
            // Note: This check can be removed if you are not using XTAL as the PLL
            // source
            //
            if( ((SYS_SETCLOCK_CFG & SYSCTL_OSCSRC_M) == SYSCTL_OSCSRC_XTAL) ||
                ((SYS_SETCLOCK_CFG & SYSCTL_OSCSRC_M) == SYSCTL_OSCSRC_XTAL_SE))
            {
                while(!Sys_VerifyXTAL(SYS_OSCSRC_FREQ / 1000000))
                {
                    //
                    // The actual XTAL frequency does not match SYS_OSCSRC_FREQ!!
                    // Please check the XTAL frequency used.
                    //
                    // By default, the Device_init function assumes 25MHz XTAL.
                    // If a 20MHz crystal is used, please add a predefined symbol
                    // "USE_20MHZ_XTAL" in your CCS project.
                    // If a different XTAL is used, please update the SYS_SETCLOCK_CFG
                    // macro accordingly.
                    //
                    // Note that the latest F2838x controlCARDs (Rev.B and later) have been
                    // updated to use 25MHz XTAL by default. If you have an older 20MHz XTAL
                    // controlCARD (E1, E2, or Rev.A), refer to the controlCARD
                    // documentation on steps to reconfigure the controlCARD from 20MHz to
                    // 25MHz.
                    //
                    SYS_DELAY_US(1000);  // Note: oscillator can take up to
                                            // 10ms to start up per data sheet
                }
            }

            //
            // Set up PLL control and clock dividers
            //
            SysCtl_setClock(SYS_SETCLOCK_CFG);

            //
            // Make sure the LSPCLK divider is set to the default (divide by 4)
            //
            SysCtl_setLowSpeedClock(SYSCTL_LSPCLK_PRESCALE_4);

            //
            // Set up AUXPLL control and clock dividers needed for CMCLK
            //
            SysCtl_setAuxClock(SYS_AUXSETCLOCK_CFG);

            //
            // Set up CMCLK to use AUXPLL as the clock source and set the
            // clock divider to 1.
            //
            SysCtl_setCMClk(SYSCTL_CMCLKOUT_DIV_1,SYSCTL_SOURCE_AUXPLL);

            // These asserts will check that the #defines for the clock rates in
            // device.h match the actual rates that have been configured. If they do
            // not match, check that the calculations of SYS_SYSCLK_FREQ,
            // SYS_LSPCLK_FREQ and SYS_AUXCLK_FREQ are accurate. Some
            // examples will not perform as expected if these are not correct.
            //
            ASSERT(SysCtl_getClock(SYS_OSCSRC_FREQ) == SYS_SYSCLK_FREQ);
            ASSERT(SysCtl_getLowSpeedClock(SYS_OSCSRC_FREQ) == SYS_LSPCLK_FREQ);
            ASSERT(SysCtl_getAuxClock(SYS_OSCSRC_FREQ) == SYS_AUXCLK_FREQ);

#ifndef _FLASH
            //
            // Call Device_cal function when run using debugger
            // This function is called as part of the Boot code. The function is called
            // in the Device_init function since during debug time resets, the boot code
            // will not be executed and the gel script will reinitialize all the
            // registers and the calibrated values will be lost.
            // Sysctl_deviceCal is a wrapper function for Device_Cal
            //
            SysCtl_deviceCal();
#endif

// Boot CPU2
#ifdef _STANDALONE
    #ifdef _FLASH
            // Send boot command to allow the CPU2 application to begin execution
            BootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH);
    #else
            // Send boot command to allow the CPU2 application to begin execution
            BootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_RAM);
    #endif // _FLASH
#endif // _STANDALONE

            // TODO - Configure XCLKOUT for measuring CPU clock
        } // CPU1

        // Init complete
        status.isInitialized = true;

    } // ConfigData


    // Init complete
    return(status.isInitialized);

}

uint32_t Sys_GetClockFrequencyHz(void)
{
    // Get the clock frequency based on the current oscillator source frequency
    return(SysCtl_getClock(SYS_OSCSRC_FREQ));
}
    
uint32_t Sys_LowSpeedClockFrequencyHz(void)
{
    // Get the low speed (peripheral) clock frequency based on the current oscillator source frequency
    return(SysCtl_getLowSpeedClock(SYS_OSCSRC_FREQ));
}

// Get version information
void Sys_MessageRouter_GetApplicationVersion(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the response.
   typedef struct
   {
      // Major version number (1 in v1.2.3)
      uint16_t major;
      // Minor version number (2 in v1.2.3)
      uint16_t minor;
      // Build number (3456 in v1.2.3456)
      uint16_t build;
   } Response_t;

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for
   // the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, 0, sizeof(Response_t)))
   {
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      // Set Major.Minor to 0.0 for Debug Targets
      if (SYS_RELEASE_CONFIGURATION_RELEASE == status.sysConfig->dataPtr[0].productConfig.releaseConfiguration)
      {
         // Release build, use version
         response->major = status.sysConfig->dataPtr[0].productConfig.version.major;
         response->minor = status.sysConfig->dataPtr[0].productConfig.version.minor;
      }
      else
      {
         // Debug Build, override version
         response->major = 0;
         response->minor = 0;
      }

      // Build is always included
      response->build = status.sysConfig->dataPtr[0].productConfig.version.build;

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

// Get product identification string
void Sys_MessageRouter_GetProductID(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the response.
   typedef struct
   {
      // 32-bit Product ID
      uint32_t productID;
   } Response_t;

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for
   // the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, 0, sizeof(Response_t)))
   {
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      // Just store the 32-bit product ID
      response->productID = status.sysConfig->dataPtr[0].productConfig.productId;

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

// Get product identification string
void Sys_MessageRouter_GetProductName(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // Response params not used -- copy string directly

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for
   // the response
   // Note that the error response will be set, if necessary
   uint16_t productNameLength = sizeof(status.sysConfig->dataPtr[0].productConfig.productName.stringValue);
   if (MessageRouter_VerifyParameterSizes(message, 0, productNameLength))
   {
      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Copy each byte to the response buffer
      for (int i = 0; i < productNameLength; i++)
      {
#if (16 == CHAR_BIT)
         // Use compiler intrinsic to copy string characters to response
         __byte(message->responseParams.data, i) = status.sysConfig->dataPtr[0].productConfig.productName.stringValue[i];
#else
         // Copy bytes directly
          message->responseParams.data[i] = status.sysConfig->dataPtr[0].productConfig.productName.stringValue[i];
#endif
      }

      // Set the response length
      message->responseParams.length = productNameLength;
   }
}

// Get the reason for the last reset
void Sys_MessageRouter_GetResetReason(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the response
   typedef struct
   {
      uint32_t resetReason;
   } Response_t;

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for
   // the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, 0, sizeof(Response_t)))
   {
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      response->resetReason = Reset_Drv_GetResetReason();

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

// Reset the system
void Sys_MessageRouter_Reset(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for
   // the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, 0, 0))
   {
      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Set the response length before calling reset
      // Depending on the implementation, the response may not go out
      MessageRouter_SetResponseSize(message, 0);

      // Just call the platform reset function
      Reset_Drv_RequestReset();
   }
}

// Get time since last reboot
void Sys_MessageRouter_GetUptimeMillseconds(MessageRouter_Message_t *const message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the response.
   typedef struct
   {
      // Elapsed time
      uint32_t uptimeMilliseconds;
   } Response_t;

   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for
   // the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, 0, sizeof(Response_t)))
   {
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Query App for the uptime
      response->uptimeMilliseconds = App_GetUptimeMilliseconds();

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}
