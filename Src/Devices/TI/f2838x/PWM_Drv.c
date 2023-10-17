/*******************************************************************************
// Includes
*******************************************************************************/
// Module Includes
#include "PWM_Drv.h"
#include "PWM_Drv_Config.h"
#include "PWM_Drv_ConfigTypes.h"
// Platform Includes
#include "MessageRouter.h"

// Other Includes
#include <stdint.h>
#include "device.h"
#include "epwm.h"
#include "gpio.h"
#include "pin_map.h"
#include "sysctl.h"

/*******************************************************************************
// Private Constant Definitions
*******************************************************************************/

// Maximum PWM value allowed for timer
#define MAX_PWM_TIMER_COUNT (65535U)

// 100KHz Clock Period
#define EPWM_TIMER_TBPRD    (1000U)


/*******************************************************************************
// Private Type Definitions
*******************************************************************************/

typedef struct
{
    uint32_t base;
    EPWM_CounterCompareModule module;

} PWM_Base_t;

typedef struct
{
   uint16_t calculatedPrescaler;
   uint16_t period;
   uint32_t periodHz;
   float dutyCycle;
   uint16_t deadtime;
   bool enableState;
} PWM_Output_t;

// This structure holds local variables for the PWM module.
typedef struct
{
    // Module Id given to this module at Initialization
    uint16_t moduleId;

    // Configuration Table passed at Initialization
    const PWM_Drv_Config_t *pwmConfig;

    // Initialization state for the module
    bool isInitialized;

    // Enable state for the module
    bool enableState;

    // Current output values for each channel
   PWM_Output_t pwmOutput[PWM_CHANNEL_COUNT];
} PWM_Status_t;



/*******************************************************************************
// Private Variable Definitions
*******************************************************************************/


static const PWM_Base_t pwmBase[PWM_CHANNEL_COUNT] =
{
   { .base = EPWM1_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM1_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM2_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM2_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM3_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM3_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM4_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM4_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM5_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM5_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM6_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM6_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM7_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM7_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM8_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM8_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM9_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM9_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM10_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM10_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM11_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM11_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM12_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM12_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM13_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM13_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM14_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM14_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM15_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM15_BASE, .module = EPWM_COUNTER_COMPARE_B },
   { .base = EPWM16_BASE, .module = EPWM_COUNTER_COMPARE_A },
   { .base = EPWM16_BASE, .module = EPWM_COUNTER_COMPARE_B },
};

// Create the local variable "status" structure.
static PWM_Status_t status;

/*******************************************************************************
// Private Function Definitions
*******************************************************************************/

#pragma FUNC_ALWAYS_INLINE(EnablePWM)
static inline void EnablePWM(uint32_t base)
{
    // Clear trip to enable PWM
    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_OST);
}

#pragma FUNC_ALWAYS_INLINE(DisablePWM)
static inline void DisablePWM(uint32_t base)
{
    // Force trip to disable PWM and set to idle
    EPWM_forceTripZoneEvent(base, EPWM_TZ_FORCE_EVENT_DCAEVT1);
}


//
// PWM trip logic configuration
//
void ConfigurePWMTrip(uint32_t base)
{
    // Enable DCAEVT1 based one-shot trip
    EPWM_enableTripZoneSignals(base, EPWM_TZ_SIGNAL_DCAEVT1);

    // What do we want the OST event to do (xA and xB both forced Low for now)
    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);

    // Clear any spurious DCAEVT1 flags
    EPWM_clearTripZoneFlag(base, EPWM_TZ_FLAG_DCAEVT1);



}

void ConfigureTrip(){

    //XBAR_INPUT1,            //!< ePWM[TZ1], ePWM[TRIP1], X-BARs, eCAPs
    XBAR_setInputPin(INPUTXBAR_BASE, XBAR_INPUT1, 56);

    // Ext Sync signal use Input 5 & 6
    //XBAR_INPUT5,            //!< EXTSYNCIN1, X-BARs, XINT2, eCAPs
    //XBAR_INPUT6,            //!< EXTSYNCIN2, ePWM[TRIP6], X-BARs, XINT3, eCAPs

    // TODO - Configure faults for Testbed
    /*
    XBAR_setInputPin(INPUTXBAR_BASE, XBAR_INPUT1, FAULT_1_GPIO);
    XBAR_setEPWMMuxConfig(XBAR_TRIP4, FAULT_1_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP4, FAULT_1_XBAR_MUX);
    XBAR_clearInputFlag(FAULT_1_XBAR_FLAG);

    XBAR_setInputPin(INPUTXBAR_BASE, XBAR_INPUT2, FAULT_2_GPIO);
    XBAR_setEPWMMuxConfig(XBAR_TRIP5, FAULT_2_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP5, FAULT_2_XBAR_MUX);
    XBAR_clearInputFlag(FAULT_2_XBAR_FLAG);

    XBAR_setInputPin(INPUTXBAR_BASE, XBAR_INPUT3, FAULT_3_GPIO);
    XBAR_setEPWMMuxConfig(XBAR_TRIP7, FAULT_3_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP7, FAULT_3_XBAR_MUX);
    XBAR_clearInputFlag(FAULT_3_XBAR_FLAG);

    XBAR_setInputPin(INPUTXBAR_BASE, XBAR_INPUT4, FAULT_4_GPIO);
    XBAR_setEPWMMuxConfig(XBAR_TRIP8, FAULT_4_XBAR_MUX_VAL);
    XBAR_enableEPWMMux(XBAR_TRIP8, FAULT_4_XBAR_MUX);
    XBAR_clearInputFlag(FAULT_4_XBAR_FLAG);
     */
}

void ConfigureAdcSync()
{
    // ADC syncs to EPWM2
    SysCtl_setSyncOutputConfig(SYSCTL_SYNC_OUT_SRC_EPWM2SYNCOUT);

    // SOCA - EXT ADC Sync not used
    SysCtl_enableExtADCSOCSource(0);
    // SOCB - EXT ADC Sync not used
    SysCtl_enableExtADCSOCSource(0);
}



static uint32_t GetPWMClockFrequency(void)
{

    // TODO - EPWM Freq 200MHz?
    // EPWM clock is max 200MHz
    // TODO - Get Frequency from System module, remove device.h
    // For now get the clock frequency based on the current oscillator source frequency
    return(SysCtl_getClock(DEVICE_OSCSRC_FREQ));
}

void InitEPWMTimer(uint32_t base)
{

    SysCtl_setEPWMClockDivider(SYSCTL_EPWMCLK_DIV_1);

    // Set-up TBCLK
    EPWM_setTimeBasePeriod(base, EPWM_TIMER_TBPRD);
    EPWM_setPhaseShift(base, 0U);
    EPWM_setTimeBaseCounter(base, 0U);

    //
    // Set Compare values
    //
    EPWM_setCounterCompareValue(base,
                                EPWM_COUNTER_COMPARE_A,
                                EPWM_TIMER_TBPRD/2);
    EPWM_setCounterCompareValue(base,
                                EPWM_COUNTER_COMPARE_B,
                                EPWM_TIMER_TBPRD/2);

    // Set up counter mode
    EPWM_setTimeBaseCounterMode(base, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_disablePhaseShiftLoad(base);
    EPWM_setPhaseShift(base, 0);
    EPWM_setClockPrescaler(base, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);

    //
    // Set up shadowing
    //
     EPWM_setCounterCompareShadowLoadMode(base,
                                          EPWM_COUNTER_COMPARE_A,
                                          EPWM_COMP_LOAD_ON_CNTR_ZERO);
    EPWM_setCounterCompareShadowLoadMode(base,
                                          EPWM_COUNTER_COMPARE_B,
                                          EPWM_COMP_LOAD_ON_CNTR_ZERO);

    // Set actions
#if 0
    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);
#endif
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    //EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    //EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
    //EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    //EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
    //EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    //EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);



    // Deadband
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, true);
    // EPWM_setRisingEdgeDelayCountShadowLoadMode(base, EPWM_RED_LOAD_ON_CNTR_ZERO);
    // 120ns Deadtime
    EPWM_setRisingEdgeDelayCount(base, 120);
    EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, true);
    //EPWM_setFallingEdgeDelayCountShadowLoadMode(base, EPWM_FED_LOAD_ON_CNTR_ZERO);
    EPWM_setFallingEdgeDelayCount(base, 120);
    //EPWM_disableRisingEdgeDelayCountShadowLoadMode(base);
    //EPWM_disableFallingEdgeDelayCountShadowLoadMode(base);

    //
    // Interrupt where we will change the Compare Values
    // Select INT on Time base counter zero event,
    // Enable INT, generate INT on 1rd event
    //
    // EPWM_setInterruptSource(base, EPWM_INT_TBCTR_ZERO);
    // EPWM_enableInterrupt(base);
    // EPWM_setInterruptEventCount(base, 1U);
}

void InitPWMMaster(uint32_t base)
{
    initEPWM(base);

    // TODO ??
    EPWM_selectPeriodLoadEvent(base, EPWM_SHADOW_LOAD_MODE_SYNC);
    EPWM_setPhaseShift(base, 0);
    EPWM_setTimeBaseCounter(base, EPWM_TIMER_TBPRD);
}
static void Init(void)
{
#ifdef CPU1
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_HRPWM);
#endif
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // Initialize device clock and peripherals
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM2);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM3);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM4);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM5);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM6);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM7);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM8);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM9);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM10);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM11);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM12);

    // Configure GPIO0/1 , GPIO2/3 and GPIO4/5 and GPIO6/7 as
    // ePWM1A/1B, ePWM2A/2B, ePWM3A/3B, ePWM4A/4B pins respectively
    ConfigureTrip();
    ConfigureAdcSync();

    // Disable sync(Freeze clock to PWM as well). GTBCLKSYNC is applicable
    // only for multiple core devices. Uncomment the below statement if
    // applicable.
    //SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_GTBCLKSYNC);
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    //uint32_t clock = Sys_GetClockFrequencyHz();
    // TODO - Get Frequency from System module, remove device.h
    // For now get the clock frequency based on the current oscillator source frequency
    uint32_t clock = SysCtl_getClock(DEVICE_SYSCLK_FREQ)/2U;

    // Initialize PWM2 without phase shift as master
    InitPWMMaster(EPWM2_BASE);

    // //Init syncronized PWMs 1,3,4
    InitPWMTimer(EPWM1_BASE);
    InitPWMTimer(EPWM3_BASE);
    InitPWMTimer(EPWM4_BASE);
    //EPWM_enableSyncOutPulseSource(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);


    EPWM_enableInterrupt(EPWM2_BASE);
    EPWM_setInterruptSource(EPWM2_BASE, EPWM_INT_TBCTR_ZERO);
    EPWM_setInterruptEventCount(EPWM2_BASE, 1);
    EPWM_enableADCTrigger(EPWM2_BASE, EPWM_SOC_A);
    EPWM_setADCTriggerSource(EPWM2_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);
    EPWM_setADCTriggerEventPrescale(EPWM2_BASE, EPWM_SOC_A, 1);

    // TODO - Sync PWM channels
    EPWM_setSyncInPulseSource(EPWM2_BASE, EPWM_SYNC_IN_PULSE_SRC_SYNCOUT_EPWM2);

    // Enable all phase shifts.
    EPWM_enablePhaseShiftLoad(EPWM1_BASE);
    EPWM_enablePhaseShiftLoad(EPWM2_BASE);
    EPWM_enablePhaseShiftLoad(EPWM3_BASE);
    EPWM_enablePhaseShiftLoad(EPWM4_BASE);
        //
    // Enable sync and clock to PWM
    //
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
    //SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_GTBCLKSYNC);


    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    //EINT;
    //ERTM;
}

/*******************************************************************************
// Public Function Definitions
*******************************************************************************/

// Module initialization
bool PWM_Drv_Init(const uint32_t moduleID, const PWM_Drv_Config_t *configPtr)
{
    // Default module to uninitialized and not enabled
    status.isInitialized = false;
    status.enableState = false;
    // Store the module Id for error reporting
    status.moduleId = moduleID;

    // First, validate the given parameter is valid
    if ((NULL != configPtr) && (NULL != configPtr->channelConfigArray))
    {
        // Store the given configuration table
        status.pwmConfig = (PWM_Drv_Config_t *)configPtr;

        // TODO
        Init();

        // After init is complete, store result
        status.isInitialized = true;
    }

    // Finally, return the result of the initialization
    return(status.isInitialized);
}

uint16_t PWM_Drv_GetDeadtimeNS(PWM_Drv_Channel_t pwmChannel)
{
    uint16_t deadtimeNS = 0U;

   // Verify the given channel
   if (pwmChannel < PWM_CHANNEL_COUNT)
   {
      // Channel is valid, convert deadtime to ticks
      // TODO: calculate conversion factor
       uint16_t deadtimeTicks = deadtimeNS * 0.2f;

      // TODO: Check calculated value
      if (deadtimeTicks > 0x4000)
      {
         // Saturate at maximum deadtime
          deadtimeTicks = 0x4000UL;
      }
      else if  (deadtimeTicks < 1U)
      {
          // Deadtime cannot be 0
          deadtimeTicks = 1U;
      }

      // Store the new deadtime
      // TODO - Store ticks or ns?
      deadtimeNS = status.pwmOutput[pwmChannel].deadtime;
   }

   // Finally, return the deadtime value
   return(deadtimeNS);
}

void PWM_Drv_SetDeadtimeNS(PWM_Drv_Channel_t pwmChannel, uint16_t deadtimeNS)
{
   // Verify the given channel
   if (pwmChannel < PWM_CHANNEL_COUNT)
   {
      // Channel is valid, convert deadtime to ticks
      // TODO: calculate conversion factor
       uint16_t deadtimeTicks = deadtimeNS * 0.2f;

      // TODO: Check calculated value
      if (deadtimeTicks > 0x4000)
      {
         // Saturate at maximum deadtime
          deadtimeTicks = 0x4000UL;
      }
      else if  (deadtimeTicks < 1U)
      {
          // Deadtime cannot be 0
          deadtimeTicks = 1U;
      }

      // Store the new deadtime
      // TODO - Store ticks or ns?
      status.pwmOutput[pwmChannel].deadtime = deadtimeTicks;

      // Set duty cycle
      // Currently, rising and falling edges are assumed symmetrical
      EPWM_setRisingEdgeDelayCount(pwmBase[pwmChannel].base, deadtimeTicks);
      EPWM_setFallingEdgeDelayCount(pwmBase[pwmChannel].base, deadtimeTicks);
   }
}



// Return stored frequency for channel
uint32_t PWM_Drv_GetFrequencyHz(PWM_Drv_Channel_t pwmChannel)
{
   uint32_t frequencyHz = 0U;

   // Verify the given channel
   if (pwmChannel < PWM_CHANNEL_COUNT)
   {
      // Channel index is valid, return the duty cycle
      frequencyHz = status.pwmOutput[pwmChannel].periodHz;
   }

   // Return the result
   return(frequencyHz);
}


// Update frequency - duty cycle will need to be recalculated ot stay the same percentage
// Dead time will not be adjusted
void PWM_Drv_SetFrequencyHz(PWM_Drv_Channel_t pwmChannel, uint32_t frequencyHz)
{
   // Make sure the channel is valid before table access
   // Frequency will be validated in the called function
   if (pwmChannel < PWM_CHANNEL_COUNT)
   {
      // Just call the combined set frequency and duty cycle function
      PWM_Drv_SetFrequencyDutyCycle(pwmChannel, frequencyHz, status.pwmOutput[pwmChannel].dutyCycle);
   }
}


// Set frequency and duty cycle based
void PWM_Drv_SetFrequencyDutyCycle(PWM_Drv_Channel_t pwmChannel, uint32_t frequencyHz, float dutyCycle)
{
   // Init to lowest prescaler
   uint16_t prescaler = 0U;

   // Verify the given channel and frequency (to prevent divide by 0
   if ((pwmChannel < PWM_CHANNEL_COUNT) && (frequencyHz > 0))
   {
      // Given channel is valid, fetch the clock frequency used for the PWM
      //prescaler = CalculatePrescaler(frequencyHz);
      prescaler = 0;


      // Store the calculated prescaler
      status.pwmOutput[pwmChannel].calculatedPrescaler = prescaler;

      // Fetch the clock frequency used for the PWM
      uint32_t div_freq = GetPWMClockFrequency();

      // Calculate 16-bit timer period based on given frequency and calculated prescaler
      uint32_t timerPeriod = (uint16_t)(div_freq / ((1U << prescaler) * frequencyHz));

      // If center-aligned, divide the period by half
      timerPeriod = timerPeriod >> 1U;

      // Verify timer calculation is valid
      if ((timerPeriod != 0U) && (timerPeriod <= MAX_PWM_TIMER_COUNT))
      {
            // Store the period for later query after adjusting for center-alignment
            status.pwmOutput[pwmChannel].period = timerPeriod;

            // Store the requested frequency in Hertz so we can return the original value
            status.pwmOutput[pwmChannel].periodHz = frequencyHz;

            // Set prescaler to the calculated value
            //SetPrescaler();

            // Calculate the new timer period
            uint32_t timerCompare = timerPeriod - (timerPeriod * dutyCycle);

            // Store the new duty cycle
            status.pwmOutput[pwmChannel].dutyCycle = dutyCycle;
            // Set the frequency
            // Complete period is 1 less than the calculated value.  timerPeriod is >0 so it is safe to subtract
            //EPWM_setTimeBasePeriod(pwmBase[pwmChannel].base, timerPeriod - 1U);
            EPWM_setTimeBasePeriod(pwmBase[pwmChannel].base, timerPeriod - 1U);
            //EPWM_setPhaseShift(pwmBase[pwmChannel].base, 0U);
            //EPWM_setTimeBaseCounter(pwmBase[pwmChannel].base, 0U);

            // Duty cycle is the channel compare value
            // EPWM_setCounterCompareValue(pwmBase[pwmChannel].base,
            //                         EPWM_COUNTER_COMPARE_A,
            //                         timerCompare);
            // EPWM_setCounterCompareValue(pwmBase[pwmChannel].base,
            //                         EPWM_COUNTER_COMPARE_B,
            //                         timerCompare);

      }
   }
}


void PWM_Drv_SetDutyCycle(PWM_Drv_Channel_t pwmChannel, float dutyCycle)
{
   // Verify the given channel
   if (pwmChannel < PWM_CHANNEL_COUNT)
   {
      // Channel is valid, verify the given duty cycle
      if (dutyCycle >= 1.0f)
      {
         // Saturate at 100%
         dutyCycle = 1.0f;
      }

      // Get the current timer period for the slice
      uint32_t timerPeriod =  EPWM_getTimeBasePeriod(pwmBase[pwmChannel].base);
      // Calculate new duty cycle based on the timer period
      uint32_t timerCompare = timerPeriod - (timerPeriod * dutyCycle);

      // Store the new duty cycle
      status.pwmOutput[pwmChannel].dutyCycle = dutyCycle;

      // Set duty cycle
      EPWM_setCounterCompareValue(pwmBase[pwmChannel].base, pwmBase[pwmChannel].module, timerCompare);
   }
}

float PWM_Drv_GetDutyCycle(PWM_Drv_Channel_t pwmChannel)
{
    uint32_t dutyCycle = 0;

   // Verify the given channel
   if (pwmChannel < PWM_CHANNEL_COUNT)
   {
      // Channel index is valid, return the duty cycle
      dutyCycle = status.pwmOutput[pwmChannel].dutyCycle;
   }

   // Return the result
   return(dutyCycle);
}

void PWM_Drv_SetPhaseShiftDegrees(PWM_Drv_Channel_t pwmChannel, float phaseDegree)
{
    uint16_t maxCnt = EPWM_TIMER_TBPRD/2;

    // Verify the given channel
    if (pwmChannel < PWM_CHANNEL_COUNT)
    {

        // Lookup PWM base
        uint32_t base = pwmBase[pwmChannel].base;

        // Change direction for negative phase offset
        if (phaseDegree < 0)
        {
            // Count down after sync
            EPWM_setPhaseShift(base, (-phaseDegree/180.0f) * maxCnt);
            EPWM_setCountModeAfterSync(base, (EPWM_SyncCountMode)1);
        }
        else
        {
            // Count up after sync
            EPWM_setPhaseShift(base, (phaseDegree/180.0f) * maxCnt);
            EPWM_setCountModeAfterSync(base, (EPWM_SyncCountMode)0);
        }
    }
}


/*******************************************************************************
// Command Processor
*******************************************************************************/

void PWM_Drv_MessageRouter_GetFrequencyHz(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------

   // This structure defines the format of the command parameters
   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
   } Command_t;

   // This structure defines the format of the response parameters
   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
      // Current frequency in Hz
      uint16_t frequencyHz;
   } Response_t;


   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), sizeof(Response_t)))
   {
      // Cast the command buffer as the command type
      Command_t *command = (Command_t *)message->commandParams.data;
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Return the requested channel index
      response->pwmChannel = command->pwmChannel;
      // Return the duty cycle for the given channel index
      response->frequencyHz = PWM_Drv_GetFrequencyHz((PWM_Drv_Channel_t)command->pwmChannel);

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}


void PWM_Drv_MessageRouter_SetFrequencyHz(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data
   typedef struct
   {
      uint16_t pwmChannel;
      // Padding for 32-bit alignment
      uint16_t dummy;
      // New frequency in Hz
      uint32_t frequencyHz;
   } Command_t;

   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), 0))
   {
      // Cast the command buffer as the command type.
      Command_t *command = (Command_t *)message->commandParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      PWM_Drv_SetFrequencyHz((PWM_Drv_Channel_t)command->pwmChannel, command->frequencyHz);

      // Set the response length
      MessageRouter_SetResponseSize(message, 0);
   }
}


void PWM_Drv_MessageRouter_GetDutyCycle(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------

   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
   } Command_t;

   // This structure defines the format of the response data
   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
       // Padding for 32-bit alignment
       uint16_t padding;
      // Current duty cycle percent
      float dutyCycle;
   }  Response_t;


   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), sizeof(Response_t)))
   {
      // Cast the command buffer as the command type
      Command_t *command = (Command_t *)message->commandParams.data;
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Return the requested channel index
      response->pwmChannel = command->pwmChannel;
      // Return the duty cycle for the given channel index
      response->dutyCycle = PWM_Drv_GetDutyCycle((PWM_Drv_Channel_t)command->pwmChannel);

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

void PWM_Drv_MessageRouter_SetDutyCycle(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data
   typedef struct
   {
      uint16_t pwmChannel;
      // Padding for 32-bit alignment
      uint16_t padding;
      // New duty cycle percent
      float dutyCycle;
   } Command_t;


   // Verify the length of the command parameters and make sure we have room for the response
   //	Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), 0))
   {
      // Cast the command buffer as the command type.
      Command_t *command = (Command_t *)message->commandParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      PWM_Drv_SetDutyCycle((PWM_Drv_Channel_t)command->pwmChannel, command->dutyCycle);

      // Set the response length
      MessageRouter_SetResponseSize(message, 0);
   }
}

void PWM_Drv_MessageRouter_GetDeadtime(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------

   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
   } Command_t;

   // This structure defines the format of the response data
   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
       // Current deadtime in nanoseconds
       uint16_t deadtimeNS;
   }  Response_t;


   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), sizeof(Response_t)))
   {
      // Cast the command buffer as the command type
      Command_t *command = (Command_t *)message->commandParams.data;
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Return the requested channel index
      response->pwmChannel = command->pwmChannel;
      // Return the duty cycle for the given channel index
      response->deadtimeNS = PWM_Drv_GetDeadtimeNS(command->pwmChannel);

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

void PWM_Drv_MessageRouter_SetDeadtime(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data
   typedef struct
   {
      uint16_t pwmChannel;
      // New duty cycle in nanoseconds
      uint16_t nanoseconds;
   } Command_t;


   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), 0))
   {
      // Cast the command buffer as the command type.
      Command_t *command = (Command_t *)message->commandParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      PWM_Drv_SetDeadtimeNS((PWM_Drv_Channel_t)command->pwmChannel, command->nanoseconds);

      // Set the response length
      MessageRouter_SetResponseSize(message, 0);
   }
}

void PWM_Drv_MessageRouter_GetEnableState(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------

   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
   } Command_t;

   // This structure defines the format of the response data
   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
      // Current channel enable state
      uint16_t enableState;
   }  Response_t;


   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), sizeof(Response_t)))
   {
      // Cast the command buffer as the command type
      Command_t *command = (Command_t *)message->commandParams.data;
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Return the requested channel index
      response->pwmChannel = command->pwmChannel;
      // TODO - Return the enable state
      response->enableState = PWM_Drv_GetDutyCycle((PWM_Drv_Channel_t)command->pwmChannel);

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

void PWM_Drv_MessageRouter_SetEnableState(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data
   typedef struct
   {
      uint16_t pwmChannel;
      // New Enable State
      uint16_t enableState;
   } Command_t;


   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), 0))
   {
      // Cast the command buffer as the command type.
      Command_t *command = (Command_t *)message->commandParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      // See if we are enabling or disabling
      if (true == (bool)command->enableState)
      {
          // We are enabling
          EnablePWM(pwmBase[command->pwmChannel].base);
      }
      else
      {
          // We are Disabling
          DisablePWM(pwmBase[command->pwmChannel].base);
      }

      // Set the response length
      MessageRouter_SetResponseSize(message, 0);
   }
}

void PWM_Drv_MessageRouter_GetPhaseOffset(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------

   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
   } Command_t;

   // This structure defines the format of the response data
   typedef struct
   {
      // PWM channel index
       uint16_t pwmChannel;
       // Current offset in degrees
       int16_t degrees;
   }  Response_t;


   //-----------------------------------------------
   // Message Processing
   //-----------------------------------------------

   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), sizeof(Response_t)))
   {
      // Cast the command buffer as the command type
      Command_t *command = (Command_t *)message->commandParams.data;
      // Cast the response buffer as the response type
      Response_t *response = (Response_t *)message->responseParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------

      // Return the requested channel index
      response->pwmChannel;
      //TODO
      response->degrees = 0;

      // Set the response length
      MessageRouter_SetResponseSize(message, sizeof(Response_t));
   }
}

void PWM_Drv_MessageRouter_SetPhaseOffset(MessageRouter_Message_t *message)
{
   //-----------------------------------------------
   // Command/Response Params
   //-----------------------------------------------
   // This structure defines the format of the command data
   typedef struct
   {
      uint16_t pwmChannel;
      // New offset in degrees
      int16_t degrees;
   } Command_t;


   // Verify the length of the command parameters and make sure we have room for the response
   //   Note that the error response will be set, if necessary
   if (MessageRouter_VerifyParameterSizes(message, sizeof(Command_t), 0))
   {
      // Cast the command buffer as the command type.
      Command_t *command = (Command_t *)message->commandParams.data;

      //-----------------------------------------------
      // Execute Command
      //-----------------------------------------------
      PWM_Drv_SetPhaseShift((PWM_Drv_Channel_t)command->pwmChannel, (float)command->degrees);

      // Set the response length
      MessageRouter_SetResponseSize(message, 0);
   }
}

