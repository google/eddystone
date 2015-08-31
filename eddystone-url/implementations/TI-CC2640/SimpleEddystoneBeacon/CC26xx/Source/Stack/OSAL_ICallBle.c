/**************************************************************************************************
  Filename:       OSAL_ICallBle.c

  Description:    This file contains function that allows user setup tasks

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
**************************************************************************************************/

/**************************************************************************************************
 *                                            INCLUDES
 **************************************************************************************************/
#include <ICall.h>
#include "hal_types.h"
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "osal_snv.h"


/* LL */
#include "ll.h"

#if defined ( OSAL_CBTIMER_NUM_TASKS )
  #include "osal_cbtimer.h"
#endif

/* L2CAP */
#include "l2cap.h"

/* gap */
#include "gap.h"

#if defined ( GAP_BOND_MGR )
  #include "gapbondmgr.h"
#endif

/* GATT */
#include "gatt.h"

/* Application */
#include "hci_tl.h"

#include "gattservapp.h"

#include "gapbondmgr.h"

#include "bleUserConfig.h"
#include "bleDispatch.h"

/*********************************************************************
 * GLOBAL VARIABLES
 */

// The order in this table must be identical to the task initialization calls below in osalInitTask.
const pTaskEventHandlerFn tasksArr[] =
{
  LL_ProcessEvent,                                                  // task 0
  HCI_ProcessEvent,                                                 // task 1
#if defined ( OSAL_CBTIMER_NUM_TASKS )
  OSAL_CBTIMER_PROCESS_EVENT( osal_CbTimerProcessEvent ),           // task 2
#endif
  L2CAP_ProcessEvent,                                               // task 3
  GAP_ProcessEvent,                                                 // task 4
  SM_ProcessEvent,                                                  // task 5
  GATT_ProcessEvent,                                                // task 6
  GATTServApp_ProcessEvent,                                         // task 7
#if defined ( GAP_BOND_MGR )
  GAPBondMgr_ProcessEvent,                                          // task 8
#endif
  bleDispatch_ProcessEvent                                          // task 9
};

const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );
uint16 *tasksEvents;

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      osalInitTasks
 *
 * @brief   This function invokes the initialization function for each task.
 *
 * @param   void
 *
 * @return  none
 */
void osalInitTasks( void )
{
  ICall_EntityID entity;
  ICall_Semaphore sem;
  uint8 taskID = 0;
  uint8 i;

  tasksEvents = (uint16 *)osal_mem_alloc( sizeof( uint16 ) * tasksCnt);
  osal_memset( tasksEvents, 0, (sizeof( uint16 ) * tasksCnt));

  /* LL Task */
  LL_Init( taskID++ );

  /* HCI Task */
  HCI_Init( taskID++ );

#if defined ( OSAL_CBTIMER_NUM_TASKS )
  /* Callback Timer Tasks */
  osal_CbTimerInit( taskID );
  taskID += OSAL_CBTIMER_NUM_TASKS;
#endif

  /* L2CAP Task */
  L2CAP_Init( taskID++ );

  /* GAP Task */
  GAP_Init( taskID++ );

  /* SM Task */
  SM_Init( taskID++ );
  
  /* GATT Task */
  GATT_Init( taskID++ );

  /* GATT Server App Task */
  GATTServApp_Init( taskID++ );
      
#if defined ( GAP_BOND_MGR )
  /* Bond Manager Task */
  GAPBondMgr_Init( taskID++ );
#endif
  
  /* ICall BLE Dispatcher Task */
  bleDispatch_Init( taskID );

  // ICall enrollment
  /* Enroll the service that this stack represents */
  ICall_enrollService(ICALL_SERVICE_CLASS_BLE, NULL, &entity, &sem);

  /* Enroll the obtained dispatcher entity and OSAL task ID of HCI Ext App
   * to OSAL so that OSAL can route the dispatcher message into
   * the appropriate OSAL task.
   */
  osal_enroll_dispatchid(taskID, entity);

  /* Register all other OSAL tasks to use the registered dispatcher entity
   * ID as the source of dispatcher messages, even though the other OSAL
   * tasks didn't register themselves to receive messages from application.
   */
  for (i = 0; i < taskID; i++)
  {
    osal_enroll_senderid(i, entity);
  }
}

/**
 * Main entry function for the stack image
 */
int stack_main( void *arg )
{
  /* User reconfiguration of BLE Controller and Host variables */
  setBleUserConfig( (bleUserCfg_t *)arg );
  
  /* Establish OSAL for a stack service that requires accompanying
   * messaging service */
  if (ICall_enrollService(ICALL_SERVICE_CLASS_BLE_MSG,
                          (ICall_ServiceFunc) osal_service_entry,
                          &osal_entity, &osal_semaphore) !=
      ICALL_ERRNO_SUCCESS)
  {
    /* abort */
    ICall_abort();
  }

  halIntState_t state;
  HAL_ENTER_CRITICAL_SECTION(state);
  
  // Turn off interrupts
  //osal_int_disable( INTS_ALL );

  // Initialize NV System
  osal_snv_init( );

  // Initialize the operating system
  osal_init_system();

  // Allow interrupts
  //osal_int_enable( INTS_ALL );
  HAL_EXIT_CRITICAL_SECTION(state);

  osal_start_system(); // No Return from here

  return 0;  // Shouldn't get here.
}

/*********************************************************************
*********************************************************************/
