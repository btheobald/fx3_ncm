#include "usb_desc.h"
#include "desc_types.h"
#include "ncm.h"
#include "net.h"
#include "struct.h"

#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyfx_config.h"
#include "cyu3usb.h"
#include "cyu3uart.h"
#include "cyu3gpio.h"
#include "cyu3utils.h"

CyU3PThread     appThread;    // Application thread structure
CyU3PThread     lwipThread;    // Application thread structure
CyU3PDmaChannel glChHandleNotifier;      // DMA MANUAL_IN channel handle.
CyU3PDmaChannel glChHandleBulkSink;      // DMA MANUAL_IN channel handle.
CyU3PDmaChannel glChHandleBulkSrc;       // DMA MANUAL_OUT channel handle.

CyU3PDmaBuffer_t notifyBuf;
CyU3PDmaBuffer_t outBuf;
CyU3PDmaBuffer_t inBuf;

CyBool_t glIsApplnActive = CyFalse;      // Whether the source sink application is active or not.
uint32_t glDMARxCount = 0;               // Counter to track the number of buffers received.
uint32_t glDMATxCount = 0;               // Counter to track the number of buffers transmitted.
CyBool_t glDataTransStarted = CyFalse;   // Whether DMA transfer has been started after enumeration.
CyBool_t StandbyModeEnable  = CyFalse;   // Whether standby mode entry is enabled.
CyBool_t TriggerStandbyMode = CyFalse;   // Request to initiate standby entry.
CyBool_t glForceLinkU2      = CyFalse;   // Whether the device should try to initiate U2 mode.

volatile uint32_t glEp0StatCount = 0;           // Number of EP0 status events received.
uint8_t glEp0Buffer[32] __attribute__ ((aligned (32))); // Local buffer used for vendor command handling.

// Control request related variables.
CyU3PEvent glBulkLpEvent;       // Event group used to signal the thread that there is a pending request.
uint32_t   gl_setupdat0;        // Variable that holds the setupdat0 value (bmRequestType, bRequest and wValue).
uint32_t   gl_setupdat1;        // Variable that holds the setupdat1 value (wIndex and wLength).
#define CYFX_USB_CTRL_TASK      (1 << 0)        // Event that indicates that there is a pending USB control request.
#define CYFX_USB_HOSTWAKE_TASK  (1 << 1)        // Event that indicates the a Remote Wake should be attempted.

// Buffer used for USB event logs.
uint8_t *gl_UsbLogBuffer = NULL;
#define CYFX_USBLOG_SIZE        (0x1000)

// Timer Instance
CyU3PTimer glLpmTimer;

// GPIO used for testing IO state retention when switching from boot firmware to full firmware.
#define FX3_GPIO_TEST_OUT               (50)
#define FX3_GPIO_TO_LOFLAG(gpio)        (1 << (gpio))
#define FX3_GPIO_TO_HIFLAG(gpio)        (1 << ((gpio) - 32))

#include <lwip/ip.h>
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/etharp.h>
#include <netif/ethernet.h>
#include <lwip/apps/lwiperf.h>

#define INIT_IP4(a, b, c, d) { PP_HTONL(LWIP_MAKEU32(a, b, c, d)) }

/* lwip context */
static struct netif netif_data;
/* shared between net_rx_cb() and service_traffic() */
static struct pbuf *received_frame;

static const ip4_addr_t ipaddr = INIT_IP4(192, 168, 7, 1);
static const ip4_addr_t netmask = INIT_IP4(255, 255, 255, 0);
static const ip4_addr_t gateway = INIT_IP4(0, 0, 0, 0);

static err_t link_output_app( struct netif *netif, struct pbuf *p ) {
    while(1) {
        if (!glIsApplnActive) {
            return ERR_USE; // Not Ready
        }
        if (net_can_tx(p->tot_len)) {
            net_tx(p, 0);
            return ERR_OK;
        }
    }
}

static err_t ip4_output_app(struct netif *netif, struct pbuf *p, const ip4_addr_t *addr) {
    return etharp_output(netif, p, addr);
}

static err_t netif_init_cb(struct netif *netif) {
    netif->mtu = 1514;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;
    netif->state = NULL;
    netif->name[0] = 'E';
    netif->name[1] = 'X';
    netif->linkoutput = link_output_app;
    netif->output = ip4_output_app;
    return ERR_OK;
}

// Application Error Handler
void CyFxAppErrorHandler(CyU3PReturnStatus_t apiRetStatus) {
    (void)(apiRetStatus);
    while(1) {
        CyU3PThreadSleep (100);
    }
}

extern uint16_t net_tx_cb( void * destination, void * datagram, uint16_t arg ) {
    (void) arg; /* unused for this example */
    struct pbuf *p = (struct pbuf *) datagram;
    return pbuf_copy_partial(p, destination, p->tot_len, 0);
}

extern uint16_t net_rx_cb( void * datagram, uint16_t size ) {
    if (received_frame) return false;
    if (size) {
        struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
        if (p) {
            /* pbuf_alloc() has already initialized struct; all we need to do is copy the data */
            memcpy(p->payload, datagram, size);
            /* store away the pointer for service_traffic() to later handle */
            received_frame = p;
        }
    }
    return true;
}

uint8_t tud_network_mac_address[6] = {0x2E, 0xF5, 0xF3, 0x68, 0xDC, 0xE6};

static void init_lwip(void) {
  struct netif *netif = &netif_data;

  lwip_init();

  netif->hwaddr_len = sizeof(tud_network_mac_address);
  memcpy(netif->hwaddr, tud_network_mac_address, sizeof(tud_network_mac_address));
  netif->hwaddr[5] ^= 0x01;

  netif = netif_add(netif, &ipaddr, &netmask, &gateway, NULL, netif_init_cb, ip_input);
  netif_set_default(netif);
}

static void service_traffic(void) {
  /* handle any packet received by tud_network_recv_cb() */
  if (received_frame) {
    ethernet_input(received_frame, &netif_data);
    pbuf_free(received_frame);
    received_frame = NULL;
    net_rx_renew();
  }
}

// This function initializes the debug module / UART
void CyFxDebugInit(void) {
    CyU3PGpioClock_t  gpioClock;
    CyU3PUartConfig_t uartConfig;
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

    /* Initialize the GPIO block. If we are transitioning from the boot app, we can verify whether the GPIO
       state is retained. */
    gpioClock.fastClkDiv = 2;
    gpioClock.slowClkDiv = 32;
    gpioClock.simpleDiv  = CY_U3P_GPIO_SIMPLE_DIV_BY_16;
    gpioClock.clkSrc     = CY_U3P_SYS_CLK_BY_2;
    gpioClock.halfDiv    = 0;
    apiRetStatus = CyU3PGpioInit (&gpioClock, NULL);

    /* When FX3 is restarting from standby mode, the GPIO block would already be ON and need not be started
       again. */
    if ((apiRetStatus != 0) && (apiRetStatus != CY_U3P_ERROR_ALREADY_STARTED)) {
        CyFxAppErrorHandler(apiRetStatus);
    } else {
        /* Set the test GPIO as an output and update the value to 0. */
        CyU3PGpioSimpleConfig_t testConf = {CyFalse, CyTrue, CyTrue, CyFalse, CY_U3P_GPIO_NO_INTR};

        apiRetStatus = CyU3PGpioSetSimpleConfig (FX3_GPIO_TEST_OUT, &testConf);
        if (apiRetStatus != 0) {
            CyFxAppErrorHandler (apiRetStatus);
        }
    }

    /* Initialize the UART for printing debug messages */
    apiRetStatus = CyU3PUartInit();
    if (apiRetStatus != CY_U3P_SUCCESS) {
        /* Error handling */
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Set UART configuration */
    CyU3PMemSet ((uint8_t *)&uartConfig, 0, sizeof (uartConfig));
    uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
    uartConfig.stopBit = CY_U3P_UART_ONE_STOP_BIT;
    uartConfig.parity = CY_U3P_UART_NO_PARITY;
    uartConfig.txEnable = CyTrue;
    uartConfig.rxEnable = CyFalse;
    uartConfig.flowCtrl = CyFalse;
    uartConfig.isDma = CyTrue;

    apiRetStatus = CyU3PUartSetConfig (&uartConfig, NULL);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Set the UART transfer to a really large value. */
    apiRetStatus = CyU3PUartTxSetBlockXfer (0xFFFFFFFF);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Initialize the debug module. */
    apiRetStatus = CyU3PDebugInit (CY_U3P_LPP_SOCKET_UART_CONS, 8);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyFxAppErrorHandler(apiRetStatus);
    }

    CyU3PDebugPreamble(CyFalse);
}


CyBool_t CyFxApplnLPMRqtCB(CyU3PUsbLinkPowerMode link_mode) {
    (void)(link_mode);
    return CyTrue;
}

/* Callback funtion for the timer expiry notification. */
void TimerCb(void) {
    /* Enable the low power mode transition on timer expiry */
    CyU3PUsbLPMEnable();
}

/* Callback funtion for the DMA event notification. */
/* *chHandle - Handle to the DMA channel. */
/* type - Callback type.             */
/* *input - Callback status.           */
void CyFxDmaCallback(CyU3PDmaChannel *chHandle, CyU3PDmaCbType_t type, CyU3PDmaCBInput_t *input) {
    (void)(input);
    CyU3PDmaBuffer_t buf_p;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    glDataTransStarted = CyTrue;

    /** Start/restart the timer and disable LPM **/
    CyU3PUsbLPMDisable();
    CyU3PTimerStop (&glLpmTimer);
    CyU3PTimerModify(&glLpmTimer, 100, 0);
    CyU3PTimerStart(&glLpmTimer);

    if (type == CY_U3P_DMA_CB_PROD_EVENT) {
        CyU3PDebugPrint (4, (char *)"PROD_DMA_CB\n\r");
        status = CyU3PDmaChannelDiscardBuffer (chHandle);
        if (status != CY_U3P_SUCCESS) {
            CyU3PDebugPrint (4, (char *)"CyU3PDmaChannelDiscardBuffer failed, Error code = %d\n", status);
        }

        /* Increment the counter. */
        glDMARxCount++;
    }
    if (type == CY_U3P_DMA_CB_CONS_EVENT) {
        CyU3PDebugPrint (4, (char *)"CONS_DMA_CB\n\r");
        status = CyU3PDmaChannelGetBuffer (chHandle, &buf_p, CYU3P_NO_WAIT);
        if (status == CY_U3P_SUCCESS) {
            /* Commit the full buffer with default status. */
            status = CyU3PDmaChannelCommitBuffer (chHandle, buf_p.size, 0);
            if (status != CY_U3P_SUCCESS) {
                CyU3PDebugPrint (4, (char *)"CyU3PDmaChannelCommitBuffer failed, Error code = %d\n", status);
            }
        }
        else {
            CyU3PDebugPrint (4, (char *)"CyU3PDmaChannelGetBuffer failed, Error code = %d\n", status);
        }

        /* Increment the counter. */
        glDMATxCount++;
    }
}

static volatile CyBool_t glSrcEpFlush = CyFalse;

void CyFxEpEvtCB(CyU3PUsbEpEvtType evtype, CyU3PUSBSpeed_t speed, uint8_t epNum) {
    (void)(speed);
    (void)(epNum);
    /* Hit an endpoint retry case. Need to stall and flush the endpoint for recovery. */
    if (evtype == CYU3P_USBEP_SS_RETRY_EVT) {
        glSrcEpFlush = CyTrue;
    }
}

/* This function starts the application. This is called
 * when a SET_CONF event is received from the USB host. The endpoints
 * are configured and the DMA pipe is setup in this function. */
void CyFxAppStart (void) {
    uint16_t size = 0;
    CyU3PEpConfig_t epCfg;
    CyU3PDmaChannelConfig_t dmaCfg;
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyU3PUSBSpeed_t usbSpeed = CyU3PUsbGetSpeed();

    /* First identify the usb speed. Once that is identified,
     * create a DMA channel and start the transfer on this. */

    /* Based on the Bus Speed configure the endpoint packet size */
    switch (usbSpeed) {
    case CY_U3P_FULL_SPEED:
        size = 64;
        CyU3PDebugPrint (4, (char *)"12M FullSpeed\n\r");
        break;

    case CY_U3P_HIGH_SPEED:
        size = 512;
        CyU3PDebugPrint (4, (char *)"480M HighSpeed\n\r");
        break;

    case  CY_U3P_SUPER_SPEED:
        size = 1024;
        CyU3PDebugPrint (4, (char *)"5G SuperSpeed\n\r");
        break;

    default:
        CyU3PDebugPrint (4, (char *)"Error! Invalid USB speed.\n");
        CyFxAppErrorHandler (CY_U3P_ERROR_FAILURE);
        break;
    }

    CyU3PMemSet ((uint8_t *)&epCfg, 0, sizeof (epCfg));
    epCfg.enable = CyTrue;
    epCfg.epType = CY_U3P_USB_EP_INTR;
    epCfg.burstLen = 1;
    epCfg.streams = 0;
    epCfg.pcktSize = 16;

    /* Notifier endpoint configuration */
    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_NOTIFIER, &epCfg);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PSetEpConfig failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    // Datapath is Bulk EP
    epCfg.epType = CY_U3P_USB_EP_BULK;

    /* Producer endpoint configuration */
    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_PRODUCER, &epCfg);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PSetEpConfig failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }
    /* Consumer endpoint configuration */
    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epCfg);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PSetEpConfig failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    /* Flush the endpoint memory */
    CyU3PUsbFlushEp(CY_FX_EP_NOTIFIER);
    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER);
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER);

    /* Create a DMA MANUAL_IN channel for the producer socket. */
    CyU3PMemSet ((uint8_t *)&dmaCfg, 0, sizeof (dmaCfg));
    dmaCfg.size = 16;
    dmaCfg.count = 1;
    dmaCfg.prodSckId = CY_U3P_CPU_SOCKET_PROD;
    dmaCfg.consSckId = CY_FX_EP_NOTIFIER_SOCKET;
    dmaCfg.dmaMode = CY_U3P_DMA_MODE_BYTE;
    dmaCfg.notification = 0;
    dmaCfg.cb = NULL;
    dmaCfg.prodHeader = 0;
    dmaCfg.prodFooter = 0;
    dmaCfg.consHeader = 0;
    dmaCfg.prodAvailCount = 0;
    apiRetStatus = CyU3PDmaChannelCreate (&glChHandleNotifier, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaCfg);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PDmaChannelCreate Notifier failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }
    CyU3PDmaChannelSetXfer (&glChHandleNotifier, 0);

    dmaCfg.size = 512;
    dmaCfg.count = 1;
    dmaCfg.prodSckId = CY_U3P_CPU_SOCKET_PROD;
    dmaCfg.consSckId = CY_FX_EP_CONSUMER_SOCKET;
    dmaCfg.cb = CyFxDmaCallback;
    apiRetStatus = CyU3PDmaChannelCreate (&glChHandleBulkSink, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaCfg);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PDmaChannelCreate Consumer failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }
    CyU3PDmaChannelSetXfer (&glChHandleBulkSink, 0);

    dmaCfg.size = 512;
    dmaCfg.count = 1;
    dmaCfg.prodSckId = CY_FX_EP_PRODUCER_SOCKET;
    dmaCfg.consSckId = CY_U3P_CPU_SOCKET_CONS;
    apiRetStatus = CyU3PDmaChannelCreate (&glChHandleBulkSrc, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaCfg);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PDmaChannelCreate Producer failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }
    CyU3PDmaChannelSetXfer (&glChHandleBulkSrc, 0);

    CyU3PUsbRegisterEpEvtCallback (CyFxEpEvtCB, CYU3P_USBEP_SS_RETRY_EVT, 0x00, 0x02);
    /* Update the flag so that the application thread is notified of this. */
    glIsApplnActive = CyTrue;
}

/* This function stops the application. This shall be called whenever a RESET
 * or DISCONNECT event is received from the USB host. The endpoints are
 * disabled and the DMA pipe is destroyed by this function. */
void CyFxAppStop(void) {
    CyU3PEpConfig_t epCfg;
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

    /* Update the flag so that the application thread is notified of this. */
    glIsApplnActive = CyFalse;

    /* Destroy the channels */
    CyU3PDmaChannelDestroy (&glChHandleNotifier);
    CyU3PDmaChannelDestroy (&glChHandleBulkSink);
    CyU3PDmaChannelDestroy (&glChHandleBulkSrc);

    /* Flush the endpoint memory */
    CyU3PUsbFlushEp(CY_FX_EP_NOTIFIER);
    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER);
    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER);

    /* Disable endpoints. */
    CyU3PMemSet ((uint8_t *)&epCfg, 0, sizeof (epCfg));
    epCfg.enable = CyFalse;

    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_NOTIFIER, &epCfg);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PSetEpConfig failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_PRODUCER, &epCfg);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PSetEpConfig failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }

    apiRetStatus = CyU3PSetEpConfig(CY_FX_EP_CONSUMER, &epCfg);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PSetEpConfig failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler (apiRetStatus);
    }
}

/* Callback to handle the USB setup requests. */
CyBool_t CyFxAppUSBSetupCB(uint32_t setupdat0, uint32_t setupdat1) {
    /* Fast enumeration is used. Only requests addressed to the interface, class,
     * vendor and unknown control requests are received by this function. */

    uint8_t  bRequest, bReqType;
    uint8_t  bType, bTarget;
    uint16_t wValue, wIndex;
    CyBool_t isHandled = CyFalse;

    /* Decode the fields from the setup request. */
    bReqType = (setupdat0 & CY_U3P_USB_REQUEST_TYPE_MASK);
    bType    = (bReqType & CY_U3P_USB_TYPE_MASK);
    bTarget  = (bReqType & CY_U3P_USB_TARGET_MASK);
    bRequest = ((setupdat0 & CY_U3P_USB_REQUEST_MASK) >> CY_U3P_USB_REQUEST_POS);
    wValue   = ((setupdat0 & CY_U3P_USB_VALUE_MASK)   >> CY_U3P_USB_VALUE_POS);
    wIndex   = ((setupdat1 & CY_U3P_USB_INDEX_MASK)   >> CY_U3P_USB_INDEX_POS);

    if (bType == CY_U3P_USB_STANDARD_RQT) {
        if ((bTarget == CY_U3P_USB_TARGET_INTF) && ((bRequest == CY_U3P_USB_SC_SET_FEATURE)
            || (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE)) && (wValue == 0)) {
            if (glIsApplnActive) {
                CyU3PUsbAckSetup ();
            } else {
                CyU3PUsbStall (0, CyTrue, CyFalse);
            }
            isHandled = CyTrue;
        }

        if ((bTarget == CY_U3P_USB_TARGET_ENDPT) && (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE)
            && (wValue == CY_U3P_USBX_FS_EP_HALT)) {
            if (glIsApplnActive) {
                if (wIndex == CY_FX_EP_PRODUCER) {
                    CyU3PUsbSetEpNak (CY_FX_EP_PRODUCER, CyTrue);
                    CyU3PBusyWait (125);

                    CyU3PDmaChannelReset (&glChHandleBulkSink);
                    CyU3PUsbFlushEp(CY_FX_EP_PRODUCER);
                    CyU3PUsbResetEp (CY_FX_EP_PRODUCER);
                    CyU3PUsbSetEpNak (CY_FX_EP_PRODUCER, CyFalse);

                    CyU3PDmaChannelSetXfer (&glChHandleBulkSink, CY_FX_BULKSRCSINK_DMA_TX_SIZE);
                    CyU3PUsbStall (wIndex, CyFalse, CyTrue);
                    isHandled = CyTrue;
                    CyU3PUsbAckSetup ();
                }

                if (wIndex == CY_FX_EP_CONSUMER) {
                    CyU3PUsbSetEpNak (CY_FX_EP_CONSUMER, CyTrue);
                    CyU3PBusyWait (125);

                    CyU3PDmaChannelReset (&glChHandleBulkSrc);
                    CyU3PUsbFlushEp(CY_FX_EP_CONSUMER);
                    CyU3PUsbResetEp (CY_FX_EP_CONSUMER);
                    CyU3PUsbSetEpNak (CY_FX_EP_CONSUMER, CyFalse);

                    CyU3PDmaChannelSetXfer (&glChHandleBulkSrc, CY_FX_BULKSRCSINK_DMA_TX_SIZE);
                    CyU3PUsbStall (wIndex, CyFalse, CyTrue);
                    isHandled = CyTrue;
                    CyU3PUsbAckSetup ();
                }
            }
        }
    }

    if (bType == CY_U3P_USB_CLASS_RQT && (bRequest == NCM_GET_NTB_PARAMETERS)) {
        isHandled = CyTrue;
        gl_setupdat0 = setupdat0;
        gl_setupdat1 = setupdat1;
        CyU3PEventSet (&glBulkLpEvent, CYFX_USB_CTRL_TASK, CYU3P_EVENT_OR);
        CyU3PDebugPrint (2, (char *)"NTB Request\r\n");
    }

    if (bType == CY_U3P_USB_STANDARD_RQT && (bRequest == USB_REQ_SET_INTERFACE)) {
        isHandled = CyTrue;
        gl_setupdat0 = setupdat0;
        gl_setupdat1 = setupdat1;
        CyU3PEventSet (&glBulkLpEvent, CYFX_USB_CTRL_TASK, CYU3P_EVENT_OR);
        CyU3PDebugPrint (2, (char *)"Set Interface Request\r\n");
    }

    if (bType == CY_U3P_USB_STANDARD_RQT && (bRequest == USB_REQ_GET_INTERFACE)) {
        isHandled = CyTrue;
        gl_setupdat0 = setupdat0;
        gl_setupdat1 = setupdat1;
        CyU3PEventSet (&glBulkLpEvent, CYFX_USB_CTRL_TASK, CYU3P_EVENT_OR);
        CyU3PDebugPrint (2, (char *)"Get Interface Request\r\n");
    }

    return isHandled;
}

/* This is the callback function to handle the USB events. */
void CyFxAppUSBEventCB(CyU3PUsbEventType_t evtype, uint16_t evdata) {
    CyU3PDebugPrint (2, (char *)"USB EVENT: %d %d\r\n", evtype, evdata);

    switch (evtype) {
    case CY_U3P_USB_EVENT_CONNECT:
        break;

    case CY_U3P_USB_EVENT_SETCONF:
        /* If the application is already active
         * stop it before re-enabling. */
        if (glIsApplnActive) {
            CyFxAppStop ();
        }

        /* Start the source sink function. */
        CyFxAppStart ();
        break;

    case CY_U3P_USB_EVENT_RESET:
    case CY_U3P_USB_EVENT_DISCONNECT:
        glForceLinkU2 = CyFalse;

        /* Stop the source sink function. */
        if (glIsApplnActive) {
            CyFxAppStop ();
        }

        glDataTransStarted = CyFalse;
        break;

    case CY_U3P_USB_EVENT_EP0_STAT_CPLT:
        glEp0StatCount++;
        CyU3PDebugPrint (4, "EP0 Status Complete %d\r\n", glEp0StatCount);
        break;

    case CY_U3P_USB_EVENT_VBUS_REMOVED:
        if (StandbyModeEnable) {
            TriggerStandbyMode = CyTrue;
            StandbyModeEnable  = CyFalse;
        }
        break;

    default:
        break;
    }
}

/* Callback function to handle LPM requests from the USB 3.0 host. This function is invoked by the API
   whenever a state change from U0 -> U1 or U0 -> U2 happens. If we return CyTrue from this function, the
   FX3 device is retained in the low power state. If we return CyFalse, the FX3 device immediately tries
   to trigger an exit back to U0.

   This application does not have any state in which we should not allow U1/U2 transitions; and therefore
   the function always return CyTrue.
 */
CyBool_t CyFxLPMCB (CyU3PUsbLinkPowerMode link_mode) {
    (void)(link_mode);
    return CyTrue;
}

/* This function initializes the USB Module, sets the enumeration descriptors.
 * This function does not start the bulk streaming and this is done only when
 * SET_CONF event is received. */
void CyFxAppInit(void) {
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;
    CyBool_t no_renum = CyFalse;

    /* The fast enumeration is the easiest way to setup a USB connection,
     * where all enumeration phase is handled by the library. Only the
     * class / vendor requests need to be handled by the application. */
    CyU3PUsbRegisterSetupCallback(CyFxAppUSBSetupCB, CyTrue);

    /* Setup the callback to handle the USB events. */
    CyU3PUsbRegisterEventCallback(CyFxAppUSBEventCB);

    /* Register a callback to handle LPM requests from the USB 3.0 host. */
    CyU3PUsbRegisterLPMRequestCallback(CyFxApplnLPMRqtCB);

    /* Start the USB functionality. */
    apiRetStatus = CyU3PUsbStart();
    if (apiRetStatus == CY_U3P_ERROR_NO_REENUM_REQUIRED) {
        no_renum = CyTrue;
    } else if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"CyU3PUsbStart failed to Start, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Change GPIO state again. */
    CyU3PGpioSimpleSetValue (FX3_GPIO_TEST_OUT, CyTrue);

    /* Super speed device descriptor. */
    // apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_DEVICE_DESCR, 0, (uint8_t *)CyFxUSB30DeviceDscr);
    // if (apiRetStatus != CY_U3P_SUCCESS) {
        // CyU3PDebugPrint (4, "USB set device descriptor failed, Error code = %d\n", apiRetStatus);
        // CyFxAppErrorHandler(apiRetStatus);
    // }

    /* High speed device descriptor. */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_DEVICE_DESCR, 0, (uint8_t *)&usb_device_desc);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"USB set device descriptor failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* BOS descriptor */
    // apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_BOS_DESCR, 0, (uint8_t *)CyFxUSBBOSDscr);
    // if (apiRetStatus != CY_U3P_SUCCESS) {
        // CyU3PDebugPrint (4, "USB set configuration descriptor failed, Error code = %d\n", apiRetStatus);
        // CyFxAppErrorHandler(apiRetStatus);
    // }

    // /* Device qualifier descriptor */
    // apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_DEVQUAL_DESCR, 0, (uint8_t *)CyFxUSBDeviceQualDscr);
    // if (apiRetStatus != CY_U3P_SUCCESS) {
        // CyU3PDebugPrint (4, "USB set device qualifier descriptor failed, Error code = %d\n", apiRetStatus);
        // CyFxAppErrorHandler(apiRetStatus);
    // }

    // /* Super speed configuration descriptor */
    // apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_CONFIG_DESCR, 0, (uint8_t *)CyFxUSBSSConfigDscr);
    // if (apiRetStatus != CY_U3P_SUCCESS) {
        // CyU3PDebugPrint (4, "USB set configuration descriptor failed, Error code = %d\n", apiRetStatus);
        // CyFxAppErrorHandler(apiRetStatus);
    // }

    /* High speed configuration descriptor */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_CONFIG_DESCR, 0, (uint8_t *)&usb_config_desc);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"USB Set Other Speed Descriptor failed, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Full speed configuration descriptor */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_FS_CONFIG_DESCR, 0, (uint8_t *)&usb_config_desc);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"USB Set Configuration Descriptor failed, Error Code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* String descriptor 0 */
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, STR_ID_LANG, (uint8_t *)&str_desc_langid);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"USB set string descriptor failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* String descriptor 1 - Manufacturer*/
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, STR_ID_MANF, (uint8_t *)&str_desc_manufacturer);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"USB set string descriptor failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* String descriptor 2 - Product*/
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, STR_ID_PROD, (uint8_t *)&str_desc_product);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"USB set string descriptor failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* String descriptor 3 - Serial / MAC*/
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, STR_ID_MACA, (uint8_t *)&str_desc_serial);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"USB set string descriptor failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* String descriptor 4 - Interface*/
    apiRetStatus = CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, STR_ID_INTF, (uint8_t *)&str_desc_interface);
    if (apiRetStatus != CY_U3P_SUCCESS) {
        CyU3PDebugPrint (4, (char *)"USB set string descriptor failed, Error code = %d\n", apiRetStatus);
        CyFxAppErrorHandler(apiRetStatus);
    }

    /* Register a buffer into which the USB driver can log relevant events. */
    gl_UsbLogBuffer = (uint8_t *)CyU3PDmaBufferAlloc (CYFX_USBLOG_SIZE);
    if (gl_UsbLogBuffer) {
        CyU3PUsbInitEventLog (gl_UsbLogBuffer, CYFX_USBLOG_SIZE);
    }

    CyU3PDebugPrint (4, (char *)"About to connect to USB host\r\n");

    if (!no_renum) {
        // Connect USB 2 Only
        apiRetStatus = CyU3PConnectState(CyTrue, CyFalse);
        if (apiRetStatus != CY_U3P_SUCCESS) {
            CyU3PDebugPrint (4, (char *)"USB Connect failed, Error code = %d\n", apiRetStatus);
            CyFxAppErrorHandler(apiRetStatus);
        }
    } else {
        /* USB connection is already active. Configure the endpoints and DMA channels. */
        CyFxAppStart();
    }

    CyU3PDebugPrint (8, (char *)"CyFxBulkSrcSinkApplnInit complete\r\n");
}

/*
 * De-initialize function for the USB block. Used to test USB Stop/Start functionality.
 */
static void CyFxAppDeinit(void) {
    if (glIsApplnActive) {
        CyFxAppStop ();
    }

    CyU3PConnectState (CyFalse, CyTrue);
    CyU3PThreadSleep (1000);
    CyU3PUsbStop ();
    CyU3PThreadSleep (1000);
}

void lwipThread_Entry() {
    net_init();
    init_lwip();
    CyU3PDebugPrint(1, (char *)"\nLwIP initialized\r\n");
    while (!netif_is_up(&netif_data));
    lwiperf_start_tcp_server_default(NULL, NULL);
    CyU3PDebugPrint(1, (char *)"\niperf initialized\r\n");
    
    while(1) {
        // Handle LwIP Traffic
        service_traffic();
    }
}

/* Entry function for the BulkSrcSinkAppThread. */
void appThread_Entry(uint32_t input) {
    (void)(input);

    CyU3PReturnStatus_t stat;
    uint32_t eventMask = CYFX_USB_CTRL_TASK | CYFX_USB_HOSTWAKE_TASK;   /* Events that we are interested in. */
    uint32_t eventStat;                                                 /* Current status of the events. */
    uint8_t  vendorRqtCnt = 0;

    uint16_t prevUsbLogIndex = 0, tmp1, tmp2;
    CyU3PUsbLinkPowerMode curState;

    /* Initialize the debug module */
    CyFxDebugInit();
    CyU3PDebugPrint(1, (char *)"\n\ndebug initialized\r\n");

    /* Initialize the application */
    CyFxAppInit();

    /* Create a timer with 100 ms expiry to enable/disable LPM transitions */ 
    CyU3PTimerCreate(&glLpmTimer, TimerCb, 0, 100, 100, CYU3P_NO_ACTIVATE);

    while(1) {
        /* The following call will block until at least one of the events enabled in eventMask is received.
           The eventStat variable will hold the events that were active at the time of returning from this API.
           The CLEAR flag means that all events will be atomically cleared before this function returns.

           We cause this event wait to time out every 10 milli-seconds, so that we can periodically get the FX3
           device out of low power modes.
           */
        stat = CyU3PEventGet (&glBulkLpEvent, eventMask, CYU3P_EVENT_OR_CLEAR, &eventStat, 10);
        if (stat == CY_U3P_SUCCESS) {
            /* If the HOSTWAKE task is set, send a DEV_NOTIFICATION (FUNCTION_WAKE) or remote wakeup signalling
               based on the USB connection speed. */
            if (eventStat & CYFX_USB_HOSTWAKE_TASK) {
                CyU3PThreadSleep (1000);
                if (CyU3PUsbGetSpeed() == CY_U3P_SUPER_SPEED) {
                    stat = CyU3PUsbSendDevNotification(1, 0, 0);
                } else {
                    stat = CyU3PUsbDoRemoteWakeup();
                }
                if (stat != CY_U3P_SUCCESS) {
                    CyU3PDebugPrint(2, (char *)"Remote wake attempt failed with code: %d\r\n", stat);
                }
            }

            /* If there is a pending control request, handle it here. */
            if (eventStat & CYFX_USB_CTRL_TASK) {
                usb_control_request_t req = {
                    .bmRequestType.direction = ((gl_setupdat0 & CY_U3P_USB_REQUEST_TYPE_MASK) & 0x80) >> 8,
                    .bmRequestType.type = ((gl_setupdat0 & CY_U3P_USB_REQUEST_TYPE_MASK) & CY_U3P_USB_TYPE_MASK) >> 5,
                    .bmRequestType.recipient = ((gl_setupdat0 & CY_U3P_USB_REQUEST_TYPE_MASK) & CY_U3P_USB_TARGET_MASK),
                    .bRequest =     ((gl_setupdat0 & CY_U3P_USB_REQUEST_MASK)   >> CY_U3P_USB_REQUEST_POS),
                    .wLength =      ((gl_setupdat1 & CY_U3P_USB_LENGTH_MASK)    >> CY_U3P_USB_LENGTH_POS),
                    .wValue =       ((gl_setupdat0 & CY_U3P_USB_VALUE_MASK)     >> CY_U3P_USB_VALUE_POS),
                    .wIndex =       ((gl_setupdat1 & CY_U3P_USB_INDEX_MASK)     >> CY_U3P_USB_INDEX_POS)
                };

                if(!net_usb_control_transfer(&req)) {
                    CyU3PUsbStall(0, CyTrue, CyFalse);
                }
            }
        }

        if (glSrcEpFlush) {
            /* Stall the endpoint, so that the host can reset the pipe and continue. */
            glSrcEpFlush = CyFalse;
            CyU3PUsbStall(CY_FX_EP_CONSUMER, CyTrue, CyFalse);
        }

        /* Force the USB 3.0 link to U2. */
        if (glForceLinkU2) {
            stat = CyU3PUsbGetLinkPowerState(&curState);
            while ((glForceLinkU2) && (stat == CY_U3P_SUCCESS) && (curState == CyU3PUsbLPM_U0)) {
                /* Repeatedly try to go into U2 state.*/
                CyU3PUsbSetLinkPowerState(CyU3PUsbLPM_U2);
                CyU3PThreadSleep (5);
                stat = CyU3PUsbGetLinkPowerState(&curState);
            }
        }

        if (TriggerStandbyMode) {
            TriggerStandbyMode = CyFalse;

            CyU3PConnectState(CyFalse, CyTrue);
            CyU3PUsbStop();
            CyU3PDebugDeInit();
            CyU3PUartDeInit();

            /* Add a delay to allow VBus to settle. */
            CyU3PThreadSleep(1000);

            /* VBus has been turned off. Go into standby mode and wait for VBus to be turned on again.
               The I-TCM content and GPIO register state will be backed up in the memory area starting
               at address 0x40060000. */
            stat = CyU3PSysEnterStandbyMode(CY_U3P_SYS_USB_VBUS_WAKEUP_SRC, CY_U3P_SYS_USB_VBUS_WAKEUP_SRC, (uint8_t *)0x40060000);
            if (stat != CY_U3P_SUCCESS) {
                CyFxDebugInit();
                CyU3PDebugPrint(4, (char *)"Enter standby returned %d\r\n", stat);
                CyFxAppErrorHandler(stat);
            }

            /* If the entry into standby succeeds, the CyU3PSysEnterStandbyMode function never returns. The
               firmware application starts running again from the main entry point. Therefore, this code
               will never be executed. */
            CyFxAppErrorHandler(1);
        } else {
            /* Compare the current USB driver log index against the previous value. */
            tmp1 = CyU3PUsbGetEventLogIndex();
            if (tmp1 != prevUsbLogIndex) {
                tmp2 = prevUsbLogIndex;
                while (tmp2 != tmp1) {
                    CyU3PDebugPrint(4, (char *)"USB LOG: %x\r\n", gl_UsbLogBuffer[tmp2]);
                    tmp2++;
                    if (tmp2 == CYFX_USBLOG_SIZE)
                        tmp2 = 0;
                }
            }

            /* Store the current log index. */
            prevUsbLogIndex = tmp1;
        }
    }
}

/* Application define function which creates the threads. */
void CyFxApplicationDefine(void) {
    void *ptr = NULL;
    void *lwip = NULL;
    uint32_t ret = CY_U3P_SUCCESS;

    /* Create an event flag group that will be used for signalling the application thread. */
    ret = CyU3PEventCreate(&glBulkLpEvent);
    if (ret != 0) {
        /* Loop indefinitely */
        while (1);
    }

    /* Allocate the memory for the threads */
    ptr = CyU3PMemAlloc (CY_FX_BULKSRCSINK_THREAD_STACK);
    
    lwip = CyU3PMemAlloc (CY_FX_BULKSRCSINK_THREAD_STACK);

    /* Create the thread for the application */
    ret = CyU3PThreadCreate (&appThread,                           /* App thread structure */
                          (char *)"21:Bulk_src_sink",                      /* Thread ID and thread name */
                          appThread_Entry,                         /* App thread entry function */
                          0,                                       /* No input parameter to thread */
                          ptr,                                     /* Pointer to the allocated thread stack */
                          CY_FX_BULKSRCSINK_THREAD_STACK,          /* App thread stack size */
                          CY_FX_BULKSRCSINK_THREAD_PRIORITY-1,       /* App thread priority */
                          CY_FX_BULKSRCSINK_THREAD_PRIORITY-1,       /* App thread priority */
                          CYU3P_NO_TIME_SLICE,                     /* No time slice for the application thread */
                          CYU3P_AUTO_START                         /* Start the thread immediately */
                          );

    /* Check the return code */
    if (ret != 0) {
        /* Thread Creation failed with the error code retThrdCreate */

        /* Add custom recovery or debug actions here */

        /* Application cannot continue */
        /* Loop indefinitely */
        while(1);
    }

    ret = CyU3PThreadCreate (&lwipThread, 
                            (char *)"22:LwIP", 
                            lwipThread_Entry, 
                            0,
                            lwip, 
                            CY_FX_BULKSRCSINK_THREAD_STACK,     
                            CY_FX_BULKSRCSINK_THREAD_PRIORITY,
                            CY_FX_BULKSRCSINK_THREAD_PRIORITY,
                            CYU3P_NO_TIME_SLICE,                
                            CYU3P_AUTO_START                    
                            );

    if (ret != 0) {
        /* Thread Creation failed with the error code retThrdCreate */

        /* Add custom recovery or debug actions here */

        /* Application cannot continue */
        /* Loop indefinitely */
        while(1);
    }

}

/*
 * Main function
 */
int main (void) {
    CyU3PIoMatrixConfig_t io_cfg;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    /* Initialize the device */
    CyU3PSysClockConfig_t clockConfig;
    clockConfig.setSysClk400  = CyFalse;
    clockConfig.cpuClkDiv     = 2;
    clockConfig.dmaClkDiv     = 2;
    clockConfig.mmioClkDiv    = 2;
    clockConfig.useStandbyClk = CyFalse;
    clockConfig.clkSrc         = CY_U3P_SYS_CLK;
    status = CyU3PDeviceInit (&clockConfig);
    if (status != CY_U3P_SUCCESS) {
        goto handle_fatal_error;
    }

    /* Initialize the caches. The D-Cache is kept disabled. Enabling this will cause performance to drop,
       as the driver will start doing a lot of un-necessary cache clean/flush operations.
       Enable the D-Cache only if there is a need to process the data being transferred by firmware code.
     */
    status = CyU3PDeviceCacheControl (CyTrue, CyFalse, CyFalse);
    if (status != CY_U3P_SUCCESS) {
        goto handle_fatal_error;
    }

    /* Configure the IO matrix for the device. On the FX3 DVK board, the COM port 
     * is connected to the IO(53:56). This means that either DQ32 mode should be
     * selected or lppMode should be set to UART_ONLY. Here we are choosing
     * UART_ONLY configuration. */
    io_cfg.isDQ32Bit = CyFalse;
    io_cfg.s0Mode = CY_U3P_SPORT_INACTIVE;
    io_cfg.s1Mode = CY_U3P_SPORT_INACTIVE;
    io_cfg.useUart   = CyTrue;
    io_cfg.useI2C    = CyFalse;
    io_cfg.useI2S    = CyFalse;
    io_cfg.useSpi    = CyFalse;
    io_cfg.lppMode   = CY_U3P_IO_MATRIX_LPP_UART_ONLY;

    /* Enable the GPIO which would have been setup by 2-stage booter. */
    io_cfg.gpioSimpleEn[0]  = 0;
    io_cfg.gpioSimpleEn[1]  = FX3_GPIO_TO_HIFLAG(FX3_GPIO_TEST_OUT);
    io_cfg.gpioComplexEn[0] = 0;
    io_cfg.gpioComplexEn[1] = 0;
    status = CyU3PDeviceConfigureIOMatrix (&io_cfg);
    if (status != CY_U3P_SUCCESS) {
        goto handle_fatal_error;
    }

    /* This is a non returnable call for initializing the RTOS kernel */
    CyU3PKernelEntry ();

    /* Dummy return to make the compiler happy */
    return 0;

handle_fatal_error:

    /* Cannot recover from this error. */
    while (1);
}

/* lwip has provision for using a mutex, when applicable */
sys_prot_t sys_arch_protect(void) {
  return 0;
}
void sys_arch_unprotect(sys_prot_t pval) {
  (void) pval;
}

/* lwip needs a millisecond time source, and the TinyUSB board support code has one available */
uint32_t sys_now(void) {
  return CyU3PGetTime();
}

bool usb_transfer_ep0 ( usb_control_request_t * request, void* buffer, uint16_t bytes ) {
    (void)(request);
    CyU3PDebugPrint(4, (char *)"NET USB TRANSFER EP0\r\n");
    CyU3PUsbSendEP0Data(bytes, buffer);
    return true;
}

bool usb_transfer_ep ( uint8_t ep_addr, void* buffer, uint16_t bytes ) {
    CyU3PDebugPrint(4, (char *)"NET USB TRANSFER EP\r\n");
    if( ep_addr == NCM_EP_NOTIFY ) {
        CyU3PDebugPrint(4, (char *)"\tNCM EP NOTIFY\r\n");
        notifyBuf.buffer = NULL;
        notifyBuf.status = 0;
        notifyBuf.size = 16;
        notifyBuf.count = bytes;
        CyU3PReturnStatus_t status;
        status = CyU3PDmaChannelGetBuffer (&glChHandleNotifier, &notifyBuf, 1000);
        if (status != CY_U3P_SUCCESS) {
            // Reset Channel and reallocate buffer
            CyU3PDmaChannelReset (&glChHandleNotifier);
            CyU3PDmaChannelSetXfer (&glChHandleNotifier, 0);
            status = CyU3PDmaChannelGetBuffer (&glChHandleNotifier, &notifyBuf, 1000);
            if (status != CY_U3P_SUCCESS) {
                return false;
            }
        }
        /* Copy Report Data into the output buffer */
        memcpy(notifyBuf.buffer, buffer, bytes);
        status = CyU3PDmaChannelCommitBuffer (&glChHandleNotifier, bytes, 0);
        if (status != CY_U3P_SUCCESS) {
            // Reset Channel
            CyU3PDmaChannelReset (&glChHandleNotifier);
            CyU3PDmaChannelSetXfer (&glChHandleNotifier, 0);
        }
        /* Wait for 2 msec after sending each packet */
        CyU3PDebugPrint (4, "\tSent Notification\r\n");
        return true;
    } if ( ep_addr == NCM_EP_DATA_OUT ) {
        CyU3PDebugPrint(4, (char *)"\tNCM EP OUT\r\n");
        outBuf.buffer = NULL;
        outBuf.status = 0;
        outBuf.size = NCM_NTB_MAX_SIZE;
        outBuf.count = bytes;
        CyU3PReturnStatus_t status;
        status = CyU3PDmaChannelGetBuffer (&glChHandleBulkSink, &outBuf, 1000);
        if (status != CY_U3P_SUCCESS) {
            // Reset Channel and reallocate buffer
            CyU3PDmaChannelReset (&glChHandleBulkSink);
            CyU3PDmaChannelSetXfer (&glChHandleBulkSink, 0);
            status = CyU3PDmaChannelGetBuffer (&glChHandleBulkSink, &outBuf, 1000);
            if (status != CY_U3P_SUCCESS) {
                return false;
            }
        }
        /* Copy Report Data into the output buffer */
        memcpy(outBuf.buffer, buffer, bytes);
        status = CyU3PDmaChannelCommitBuffer (&glChHandleBulkSink, 2, 0);
        if (status != CY_U3P_SUCCESS) {
            // Reset Channel
            CyU3PDmaChannelReset (&glChHandleBulkSink);
            CyU3PDmaChannelSetXfer (&glChHandleBulkSink, 0);
        }
    } if ( ep_addr == NCM_EP_DATA_IN ) {
        CyU3PDebugPrint(4, (char *)"\tNCM EP IN\r\n");
    }
    
    return false;
}

bool usb_control_status( usb_control_request_t * request ) {
    CyU3PDebugPrint(4, (char *)"NET USB CONTROL STATUS\r\n");
    CyU3PUsbAckSetup();
    return true;
}

bool usb_ep_busy ( uint8_t ep_addr ) {
    CyU3PDebugPrint(4, (char *)"NET USB EP BUSY\r\n");

    
    return false;
}

int memcmp(const void *vl, const void *vr, size_t n) {
	const unsigned char *l=vl, *r=vr;
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l-*r : 0;
}