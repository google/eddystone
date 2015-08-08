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
 *      nvm_access.c
 *
 *  DESCRIPTION
 *      This file defines routines used by application to access NVM.
 *
 
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <pio.h>            /* PIO configuration and control functions */
#include <nvm.h>            /* Access to Non-Volatile Memory */
#include <i2c.h>            /* Access to I2C bus */
#include <panic.h>          /* Support for applications to panic */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "nvm_access.h"     /* Interface to this file */
#include "esurl_beacon.h"    /* Definitions used throughout the GATT server */

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Disable
 *
 *  DESCRIPTION
 *      This function is used to perform the actions necessary to save power on
 *      NVM once read/write operations are done.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void Nvm_Disable(void)
{
    /* Disable the NVM. */
    NvmDisable();

    /* Pull down the I2C lines to save power. */
    PioSetI2CPullMode(pio_i2c_pull_mode_strong_pull_down);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Read
 *
 *  DESCRIPTION
 *      Read words from the NVM Store after preparing the NVM to be readable. 
 *      After the read operation, perform the actions necessary to save power
 *      on NVM.
 *
 *      Read words starting at the word offset, and store them in the supplied
 *      buffer.
 *
 *  PARAMETERS
 *      buffer [out]            Data read from NVM
 *      length [in]             Number of words of data to read
 *      offset [in]             Offset from which to start reading, in words
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void Nvm_Read(uint16 *buffer, uint16 length, uint16 offset)
{
    sys_status result;

    /* Read from NVM. Firmware re-enables the NVM if it is disabled */
    result = NvmRead(buffer, length, offset);

    /* Disable NVM to save power after read operation */
    Nvm_Disable();

    /* Report panic if NVM read is not successful */
    if(sys_status_success != result)
    {
        ReportPanic(app_panic_nvm_read);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      Nvm_Write
 *
 *  DESCRIPTION
 *      Write words to the NVM Store after preparing the NVM to be writable. 
 *      After the write operation, perform the actions necessary to save power
 *      on NVM.
 *
 *      Write words from the supplied buffer into the NVM Store, starting at the
 *      word offset.
 *
 *  PARAMETERS
 *      buffer [in]             Data to write to NVM
 *      length [in]             Number of words of data to write
 *      offset [in]             Offset from which to start writing, in words
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
void Nvm_Write(uint16 *buffer, uint16 length, uint16 offset)
{
    sys_status result;          /* Function status */

    /* Write to NVM. Firmware re-enables the NVM if it is disabled */
    result = NvmWrite(buffer, length, offset);

    /* Disable NVM to save power after write operation */
    Nvm_Disable();

    /* Report panic if NVM write is not successful */
    if(sys_status_success != result)
    {
        ReportPanic(app_panic_nvm_write);
    }
}
