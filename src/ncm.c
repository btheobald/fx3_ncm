#include <stdint.h>
#include <stdbool.h>
#include "ncm.h"
#include "desc_value.h"

typedef struct {
  uint8_t alt_selected;

  __attribute__ ((aligned (32))) rx_ntb_t rx_ntb[NCM_OUT_NTB_N];
  rx_ntb_t *rx_free_ntb[NCM_OUT_NTB_N];     // NTBs Avaliable
  rx_ntb_t *rx_ready_ntb[NCM_OUT_NTB_N];    // NTBs Waiting for Glue
  rx_ntb_t *rx_usb_ntb;                     // NTB USB -> Driver
  rx_ntb_t *rx_glue_ntb;                    // NTB Driver -> Glue
  uint16_t rx_glue_ntb_datagram_ndx;        // Currently Processing Datagram

  __attribute__ ((aligned (32))) tx_ntb_t tx_ntb[NCM_IN_NTB_N];
  tx_ntb_t *tx_free_ntb[NCM_IN_NTB_N];      // NTBs Avaliable
  tx_ntb_t *tx_ready_ntb[NCM_IN_NTB_N];     // NTBs Waiting for USB
  tx_ntb_t *tx_usb_ntb;                     // NTB Driver -> USB
  tx_ntb_t *tx_glue_ntb;                    // NTB Glue -> Driver
  uint16_t tx_glue_ntb_datagram_ndx;        // Currently Processing Datagram

  bool notification_tx_is_running;          // Notification Transmitting
} ncm_interface_t;

const ntb_parameters_t ntb_parameters = {
    .wLength                  = sizeof(ntb_parameters_t),
    .bmNtbFormatsSupported    = 0x01,// 16-bit NTB supported
    .dwNtbInMaxSize           = NCM_IN_NTB_MAX_SIZE,
    .wNdpInDivisor            = 1,
    .wNdpInPayloadRemainder   = 0,
    .wNdpInAlignment          = NCM_NDP_ALIGN_MIN_SIZE,
    .wReserved                = 0,
    .dwNtbOutMaxSize          = NCM_NDP_ALIGN_MIN_SIZE,
    .wNdpOutDivisor           = 1,
    .wNdpOutPayloadRemainder  = 0,
    .wNdpOutAlignment         = NCM_NDP_ALIGN_MIN_SIZE,
    .wNtbOutMaxDatagrams      = 6,
};
 
usb_cdc_notification_t notify_connection = {
    .bmRequestType = { USB_DATA_DEVICE_TO_HOST, USB_TYPE_CLASS, USB_RECIP_INTERFACE },
    .bNotificationType = USB_CDC_NOTIFY_NETWORK_CONNECTION,
    .wValue = 1,
    .wLength = 0
};

usb_cdc_notify_speed_t notify_speed = {
    .header = {
        .bmRequestType = { USB_DATA_DEVICE_TO_HOST, USB_TYPE_CLASS, USB_RECIP_INTERFACE },
        .bNotificationType = USB_CDC_NOTIFY_SPEED_CHANGE,
        .wLength = 8
    },
    .DLBitRRate = 480000000,
    .ULBitRate = 480000000
};