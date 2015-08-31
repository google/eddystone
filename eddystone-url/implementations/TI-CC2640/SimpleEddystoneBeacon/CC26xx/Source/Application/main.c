/**
  @file  main.c
  @brief main entry of the BLE stack sample application.

* Copyright (c) 2015, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* *  Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* *  Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* *  Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <xdc/runtime/Error.h>

#include <ti/sysbios/family/arm/cc26xx/Power.h>
#include <ti/sysbios/BIOS.h>

#include "ICall.h"
#include "bcomdef.h"
#include "peripheral.h"
#include "simpleEddystoneBeacon.h"

/* Header files required to enable instruction fetch cache */
#include <inc/hw_memmap.h>
#include <driverlib/vims.h>

#ifndef USE_DEFAULT_USER_CFG

#include "bleUserConfig.h"

// BLE user defined configuration
bleUserCfg_t user0Cfg = BLE_USER_CFG;

#endif // USE_DEFAULT_USER_CFG

/**
 * Exception handler
 */
void exceptionHandler()
{
	volatile uint8_t i = 1;
    while(i){}
}

#ifdef FEATURE_OAD
#if defined(__IAR_SYSTEMS_ICC__)
extern uint32_t __vector_table;
#elif defined (__TI_COMPILER_VERSION__)
extern uint32_t ti_sysbios_family_arm_m3_Hwi_resetVectors;
#endif //Compiler
#endif //FEATURE_OAD

/*
 *  ======== main ========
 */
int main()
{
  PIN_init(BoardGpioInitTable);

#ifndef POWER_SAVING
    /* Set constraints for Standby, powerdown and idle mode */
    Power_setConstraint(Power_SB_DISALLOW);
    Power_setConstraint(Power_IDLE_PD_DISALLOW);
#endif // POWER_SAVING
    
    /* Initialize ICall module */
    ICall_init();

    /* Start tasks of external images - Priority 5 */
    ICall_createRemoteTasks();
    
    /* Kick off profile - Priority 3 */
    GAPRole_createTask();
    
    SimpleEddystoneBeacon_createTask();

#ifdef FEATURE_OAD
    {
      uint8_t counter;
      uint32_t *vectorTable =  (uint32_t*) 0x20000000;
#if defined(__IAR_SYSTEMS_ICC__)
      uint32_t *flashVectors = &__vector_table;
#elif defined(__TI_COMPILER_VERSION__)
      uint32_t *flashVectors = &ti_sysbios_family_arm_m3_Hwi_resetVectors;
#endif //Compiler.
      
      // Write image specific interrupt vectors into RAM vector table.
      for(counter = 0; counter < 15; ++counter)
      {
        *vectorTable++ = *flashVectors++;
      }
    }
#endif //FEATURE_OAD
    
    /* enable interrupts and start SYS/BIOS */
    BIOS_start();
    
    return 0;
}

/**
 * Error handled to be hooked into TI-RTOS
 */
Void smallErrorHook(Error_Block *eb)
{
  for (;;);
}

/**
 * HAL assert handler required by OSAL memory module.
 */
void halAssertHandler(void)
{
  for (;;);
}
