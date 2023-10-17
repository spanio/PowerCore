#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
   PWM_DRV_CHANNEL_ID_GATE_PRI_HB1_LS, // EPWM1A
   PWM_DRV_CHANNEL_ID_GATE_PRI_HB1_HS, // EPWM1B
   PWM_DRV_CHANNEL_ID_GATE_PRI_HB2_LS, // EPWM2A
   PWM_DRV_CHANNEL_ID_GATE_PRI_HB2_HS, // EPWM2B
   PWM_DRV_CHANNEL_ID_GATE_SEC_HB1_LS, // EPWM3A
   PWM_DRV_CHANNEL_ID_GATE_SEC_HB1_HS, // EPWM3B
   PWM_DRV_CHANNEL_ID_GATE_SEC_HB2_LS, // EPWM4A
   PWM_DRV_CHANNEL_ID_GATE_SEC_HB2_HS, // EPWM4B
    // Number of PWM channels
   PWM_CHANNEL_COUNT
} PWM_Drv_Channel_t;

typedef enum
{
    PWM_GROUP_COUNT
} PWM_Drv_Group_t;



#ifdef __cplusplus
extern "C"
}

#endif

