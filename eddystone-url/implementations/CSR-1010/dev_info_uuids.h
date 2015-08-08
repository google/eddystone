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
 *      dev_info_uuids.h
 *
 *  DESCRIPTION
 *      UUID MACROs for the Device Information Service
 *
 
 *
 *****************************************************************************/

#ifndef __DEV_INFO_UUIDS_H__
#define __DEV_INFO_UUIDS_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Brackets should not be used around the values of these macros. This file is
 * imported by the GATT Database Generator (gattdbgen) which does not understand 
 * brackets and will raise syntax errors.
 */

/* For UUID values, refer http://developer.bluetooth.org/gatt/services/
 * Pages/ServiceViewer.aspx?u=org.bluetooth.service.device_information.xml
 */

#define UUID_DEVICE_INFO_SERVICE                         0x180A

#define UUID_DEVICE_INFO_SYSTEM_ID                       0x2A23

#define UUID_DEVICE_INFO_MODEL_NUMBER                    0x2A24

#define UUID_DEVICE_INFO_SERIAL_NUMBER                   0x2A25

#define UUID_DEVICE_INFO_HARDWARE_REVISION               0x2A27

#define UUID_DEVICE_INFO_FIRMWARE_REVISION               0x2A26

#define UUID_DEVICE_INFO_SOFTWARE_REVISION               0x2A28

#define UUID_DEVICE_INFO_MANUFACTURER_NAME               0x2A29

#define UUID_DEVICE_INFO_PNP_ID                          0x2A50

/* Vendor ID Source */
#define VENDOR_ID_SRC_BT                                 0x01
#define VENDOR_ID_SRC_USB                                0x02

/* Vendor ID - CSR */
#define VENDOR_ID                                        0x000A
#define PRODUCT_ID                                       0x014C
#define PRODUCT_VER                                      0x0100

#if defined(CSR101x_A05)
    #define HARDWARE_REVISION "CSR101x A05"
#elif defined(CSR100x)
    #define HARDWARE_REVISION "CSR100x A04"
#else
    #define HARDWARE_REVISION "Unknown"
#endif

#endif /* __DEV_INFO_UUIDS_H__ */
