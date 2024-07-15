/*
 ## Cypress USB 3.0 Platform header file (cyfxbulksrcsink.h)
 ## ===========================
 ##
 ##  Copyright Cypress Semiconductor Corporation, 2010-2023,
 ##  All Rights Reserved
 ##  UNPUBLISHED, LICENSED SOFTWARE.
 ##
 ##  CONFIDENTIAL AND PROPRIETARY INFORMATION
 ##  WHICH IS THE PROPERTY OF CYPRESS.
 ##
 ##  Use of this file is governed
 ##  by the license agreement included in the file
 ##
 ##     <install>/license/license.txt
 ##
 ##  where <install> is the Cypress software
 ##  installation root directory path.
 ##
 ## ===========================
*/

/* This file contains the constants used by the bulk source sink application example */

#ifndef _INCLUDED_CYFXBULKSRCSINK_H_
#define _INCLUDED_CYFXBULKSRCSINK_H_

#include "cyu3types.h"
#include "cyu3usbconst.h"

#define CY_FX_EP_NOTIFIER_SOCKET        CY_U3P_UIB_SOCKET_CONS_1    /* Socket 1 is consumer */
#define CY_FX_EP_PRODUCER_SOCKET        CY_U3P_UIB_SOCKET_PROD_2    /* Socket 2 is producer */
#define CY_FX_EP_CONSUMER_SOCKET        CY_U3P_UIB_SOCKET_CONS_2    /* Socket 2 is consumer */

#define CY_FX_BULKSRCSINK_DMA_TX_SIZE        (0)        /* DMA transfer size is set to infinite */
#define CY_FX_BULKSRCSINK_THREAD_STACK       (0x1000)   /* Bulk loop application thread stack size */
#define CY_FX_BULKSRCSINK_THREAD_PRIORITY    (8)        /* Bulk loop application thread priority */

/* Burst length in 1 KB packets. Only applicable to USB 3.0. */
#define CY_FX_EP_BURST_LENGTH                   (16)

/* Multiplication factor used when allocating DMA buffers to reduce DMA callback frequency. */
#define CY_FX_DMA_SIZE_MULTIPLIER               (2)

/* Number of DMA buffers to be used. More buffers can give better throughput. */
#define CY_FX_BULKSRCSINK_DMA_BUF_COUNT         (3)

/* Byte value that is filled into the source buffers that FX3 sends out. */
#define CY_FX_BULKSRCSINK_PATTERN            (0xAA)

#endif /* _INCLUDED_CYFXBULKSRCSINK_H_ */

/*[]*/
