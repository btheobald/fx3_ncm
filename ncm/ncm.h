#pragma once
#include <stdint.h>

#define NCM_EP_SIZE 512

#define NCM_EP_INTERRUPT  0x81
#define NCM_EP_DATA_OUT   0x02
#define NCM_EP_DATA_IN    0x82

#define NCM_NTB_MAX_SIZE 3200
#define NCM_NTB_N 2
#define NCM_MAX_DATAGRAMS_PER_NTB 8
#define NCM_NDP_ALIGN_MIN_SIZE 4

#define NTH16_SIGNATURE 0x484D434E
#define NDP16_SIGNATURE_NCM0 0x304D434E
#define NDP16_SIGNATURE_NCM1 0x314D434E

#define USB_CDC_NOTIFY_NETWORK_CONNECTION	0x00
#define USB_CDC_NOTIFY_RESPONSE_AVAILABLE	0x01
#define USB_CDC_NOTIFY_SERIAL_STATE		0x20
#define USB_CDC_NOTIFY_SPEED_CHANGE		0x2a

// Table 6.2 Class-Specific Request Codes for Network Control Model subclass
typedef enum {
  NCM_SET_ETHERNET_MULTICAST_FILTERS               = 0x40,
  NCM_SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x41,
  NCM_GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x42,
  NCM_SET_ETHERNET_PACKET_FILTER                   = 0x43,
  NCM_GET_ETHERNET_STATISTIC                       = 0x44,
  NCM_GET_NTB_PARAMETERS                           = 0x80,
  NCM_GET_NET_ADDRESS                              = 0x81,
  NCM_SET_NET_ADDRESS                              = 0x82,
  NCM_GET_NTB_FORMAT                               = 0x83,
  NCM_SET_NTB_FORMAT                               = 0x84,
  NCM_GET_NTB_INPUT_SIZE                           = 0x85,
  NCM_SET_NTB_INPUT_SIZE                           = 0x86,
  NCM_GET_MAX_DATAGRAM_SIZE                        = 0x87,
  NCM_SET_MAX_DATAGRAM_SIZE                        = 0x88,
  NCM_GET_CRC_MODE                                 = 0x89,
  NCM_SET_CRC_MODE                                 = 0x8A,
} ncm_request_code_t;

typedef enum {
  NOTIFICATION_SPEED,
  NOTIFICATION_CONNECTED,
  NOTIFICATION_DONE
} notification_tx_state_t;            // state of notification transmission

typedef struct {
  uint16_t wLength;
  uint16_t bmNtbFormatsSupported;
  uint32_t dwNtbInMaxSize;
  uint16_t wNdpInDivisor;
  uint16_t wNdpInPayloadRemainder;
  uint16_t wNdpInAlignment;
  uint16_t wReserved;
  uint32_t dwNtbOutMaxSize;
  uint16_t wNdpOutDivisor;
  uint16_t wNdpOutPayloadRemainder;
  uint16_t wNdpOutAlignment;
  uint16_t wNtbOutMaxDatagrams;
} __attribute__ ((packed)) ntb_parameters_t;

typedef struct {
  uint32_t dwSignature;
  uint16_t wHeaderLength;
  uint16_t wSequence;
  uint16_t wBlockLength;
  uint16_t wNdpIndex;
} __attribute__ ((packed)) nth16_t;

typedef struct {
  uint16_t wDatagramIndex;
  uint16_t wDatagramLength;
} __attribute__ ((packed)) ndp16_datagram_t;

typedef struct {
  uint32_t dwSignature;
  uint16_t wLength;
  uint16_t wNextNdpIndex;
  //ndp16_datagram_t datagram[];
} __attribute__ ((packed)) ndp16_t;

typedef union {
  struct {
    nth16_t nth;
    ndp16_t ndp;
    ndp16_datagram_t ndp_datagram[NCM_MAX_DATAGRAMS_PER_NTB + 1];
  };
  uint8_t data[NCM_NTB_MAX_SIZE];
} __attribute__ ((packed)) ntb_def_t;

typedef union {
  struct {
    nth16_t nth;
    // Received Data may be algined differently
  };
  uint8_t data[NCM_NTB_MAX_SIZE];
} __attribute__ ((packed)) ntb_t;

typedef struct {
	__attribute__ ((packed)) struct {
    uint8_t transfer_direction : 1;
	  uint8_t type : 2;
	  uint8_t recipient : 5;
  } bmRequestType;
	uint8_t	  bNotificationType;
	uint16_t	wValue;
	uint16_t	wIndex;
	uint16_t	wLength;
} __attribute__ ((packed)) usb_cdc_notification_t;

typedef struct {
	usb_cdc_notification_t header;
	uint32_t	DLBitRRate;	/* contains the downlink bit rate (IN pipe) */
	uint32_t	ULBitRate;	/* contains the uplink bit rate (OUT pipe) */
} __attribute__ ((packed)) usb_cdc_notify_speed_t;