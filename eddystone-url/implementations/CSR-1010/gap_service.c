/******************************************************************************
 *    Copyright (c) 2015 Cambridge Silicon Radio Limited 
 *    All rights reserved.
 * 
 *    Redistribution and use in source and binary forms, with or without modification, 
 *    are permitted (subject to the limitations in the disclaimer below) provided that the
 *    following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice, this list of 
 *    conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 *    and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * 
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  FILE
 *      gap_service.c
 *
 *  DESCRIPTION
 *      This file defines routines for using the GAP Service.
 *
 
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <gatt.h>           /* GATT application interface */
#include <mem.h>            /* Memory library */
#include <buf_utils.h>      /* Buffer functions */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "gatt_access.h"    /* GATT-related routines */
#include "gap_service.h"    /* Interface to this file */
#include "app_gatt_db.h"    /* GATT database definitions */
#include "nvm_access.h"     /* Non-volatile memory access */

/*============================================================================*
 *  Private Data Types
 *============================================================================*/

/* GAP Service data type */
typedef struct _GAP_DATA_T
{
    /* Name length in bytes */
    uint16  length;

    /* Pointer to hold device name used by the application */
    uint8   *p_dev_name;

    /* NVM offset at which GAP Service data is stored */
    uint16  nvm_offset;

} GAP_DATA_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* GAP Service data instance */
static GAP_DATA_T g_gap_data;

/* Default device name - added two for storing AD Type and Null ('\0') */ 
static uint8 g_device_name[DEVICE_NAME_MAX_LENGTH + 2] = {
    AD_TYPE_LOCAL_NAME_COMPLETE, 
    'E', 'S', ' ', 'C', 'o', 'n', 'f', 'i', 'g', ' ', 'U', 'R', 'L', '\0'};

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Number of words of NVM memory used by the GAP Service */

/* Add space for Device Name Length and Device Name */
#define GAP_SERVICE_NVM_MEMORY_WORDS  (DEVICE_NAME_MAX_LENGTH + 2)

/* The offset of data being stored in NVM for the GAP Service. This offset is 
 * added to the GAP Service offset in the NVM region (see g_gap_data.nvm_offset) 
 * to get the absolute offset at which this data is stored in NVM.
 */
#define GAP_NVM_DEVICE_LENGTH_OFFSET  (0)

#define GAP_NVM_DEVICE_NAME_OFFSET    (1)

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Write the device name into NVM */
static void gapWriteDeviceNameToNvm(void);

/* Update the device name to the new requested value */
static void updateDeviceName(uint16 length, uint8 *name);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      gapWriteDeviceNameToNvm
 *
 *  DESCRIPTION
 *      This function is used to write GAP Device Name Length and Device Name 
 *      to NVM.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void gapWriteDeviceNameToNvm(void)
{

    /* Write device name length to NVM */
    Nvm_Write(&g_gap_data.length, sizeof(g_gap_data.length), 
              g_gap_data.nvm_offset + 
              GAP_NVM_DEVICE_LENGTH_OFFSET);

    /* Write device name to NVM 
     * Typecasting uint8 to uint16 or vice-versa does not have any side effects
     * as both types (uint8 and uint16) take one word of memory on the XAP
     */
    Nvm_Write((uint16*)g_gap_data.p_dev_name, g_gap_data.length, 
              g_gap_data.nvm_offset + 
              GAP_NVM_DEVICE_NAME_OFFSET);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      updateDeviceName
 *
 *  DESCRIPTION
 *      This function updates the device name and length in the GAP Service.
 *
 *  PARAMETERS
 *      length [in]             Device name length
 *      name [in]               Device name
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void updateDeviceName(uint16 length, uint8 *name)
{
    /* Pointer to the device name in local storage */
    uint8   *p_name = g_gap_data.p_dev_name;
    
    /* Update device name length limited to a maximum of
     * DEVICE_NAME_MAX_LENGTH */
    if(length < DEVICE_NAME_MAX_LENGTH)
        g_gap_data.length = length;
    else
        g_gap_data.length = DEVICE_NAME_MAX_LENGTH;

    /* Update device name */
    MemCopy(p_name, name, g_gap_data.length);

    /* Null terminate the device name string */
    p_name[g_gap_data.length] = '\0';

    /* Write updated device name to NVM */
    gapWriteDeviceNameToNvm();

}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      GapDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialise the GAP Service data structure.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GapDataInit(void)
{
    /* Skip first byte to move over AD Type field and point to device name */
    g_gap_data.p_dev_name = (g_device_name + 1);
    g_gap_data.length = StrLen((char *)g_gap_data.p_dev_name);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GapHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operations on GAP Service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP
 *      message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GapHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16 length = 0;                  /* Length of attribute data, octets */
    uint8 *p_value = NULL;              /* Pointer to attribute value */
    sys_status rc = sys_status_success; /* Function status */

    switch(p_ind->handle)
    {

        case HANDLE_DEVICE_NAME:
        {
            /* Validate offset against length, it should be less than the
             * device name length
             */
            if(p_ind->offset < g_gap_data.length)
            {
                length = g_gap_data.length - p_ind->offset;
                p_value = g_gap_data.p_dev_name + p_ind->offset;
            }
            else
            {
                rc = gatt_status_invalid_offset;
            }
        }
        break;

        default:
            /* No more IRQ characteristics */
            rc = gatt_status_read_not_permitted;
        break;

    }

    /* Send the GATT response. */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, length, p_value);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GapHandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles write operations on GAP Service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP
 *      message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GapHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    sys_status rc = sys_status_success; /* Function status */

    switch(p_ind->handle)
    {

        case HANDLE_DEVICE_NAME:
            /* Update device name */
            updateDeviceName(p_ind->size_value, p_ind->value);
        break;

        default:
            /* No more IRQ characteristics */
            rc = gatt_status_write_not_permitted;
        break;

    }

    /* Send the GATT response. */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GapReadDataFromNVM
 *
 *  DESCRIPTION
 *      This function is used to read GAP Service specific data stored in NVM.
 *
 *  PARAMETERS
 *      p_offset [in]           Offset to GAP Service data in NVM
 *               [out]          Offset to next entry in NVM
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GapReadDataFromNVM(uint16 *p_offset)
{

    g_gap_data.nvm_offset = *p_offset;

    /* Read Device Length */
    Nvm_Read(&g_gap_data.length, sizeof(g_gap_data.length), 
             *p_offset + 
             GAP_NVM_DEVICE_LENGTH_OFFSET);

    /* Read Device Name
     * Typecasting uint8 to uint16 or vice-versa does not have any side effects
     * as both types (uint8 and uint16) take one word of memory on the XAP
     */
    Nvm_Read((uint16*)g_gap_data.p_dev_name, g_gap_data.length, 
             *p_offset + 
             GAP_NVM_DEVICE_NAME_OFFSET);

    /* Add NUL character to terminate the device name string */
    g_gap_data.p_dev_name[g_gap_data.length] = '\0';

    /* Increase NVM offset for maximum device name length. Add 1 for
     * 'device name length' field as well
     */
    *p_offset += DEVICE_NAME_MAX_LENGTH + 1;

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GapInitWriteDataToNVM
 *
 *  DESCRIPTION
 *      This function is used to write GAP Service specific data to NVM for the
 *      first time during application initialisation.
 *
 *  PARAMETERS
 *      p_offset [in]           Offset to GAP Service data in NVM
 *               [out]          Offset to next entry in NVM
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void GapInitWriteDataToNVM(uint16 *p_offset)
{

    /* The NVM offset at which GAP Service data will be stored */
    g_gap_data.nvm_offset = *p_offset;

    /* Write device name to NVM */
    gapWriteDeviceNameToNvm();

    /* Increase NVM offset for maximum device name length. Add 1 for
     * 'device name length' field as well
     */
    *p_offset += DEVICE_NAME_MAX_LENGTH + 1;

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GapCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the GAP Service.
 *
 *  PARAMETERS
 *      handle [in]             Handle to check
 *
 *  RETURNS
 *      TRUE if handle belongs to the GAP Service, FALSE otherwise
 *----------------------------------------------------------------------------*/
extern bool GapCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_GAP_SERVICE) &&
            (handle <= HANDLE_GAP_SERVICE_END))
            ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GapGetNameAndLength
 *
 *  DESCRIPTION
 *      This function is used to get the reference to the 'g_device_name' array, 
 *      which contains AD Type and device name.
 *
 *  PARAMETERS
 *      p_name_length [out]     Device name length
 *
 *  RETURNS
 *      Device name array, including AD Type and device name.
 *----------------------------------------------------------------------------*/
extern uint8 *GapGetNameAndLength(uint16 *p_name_length)
{
    /* Update the device name length. */
    *p_name_length = StrLen((char *)g_device_name);

    /* Return the device name pointer */
    return g_device_name;
}
