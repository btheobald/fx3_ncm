#pragma once

#include <stdint.h>
#define USB_MS_TO_HS_INTERVAL(x)	(ilog2((x * 1000 / 125)) + 1)

typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
} __attribute__ ((packed)) usb_descriptor_header_t;

typedef struct {
    uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bString[31];	// Wasteful Allocation for strings
} __attribute__ ((packed)) usb_string_descriptor_t;

typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdUSB;
	uint8_t  bDeviceClass;
	uint8_t  bDeviceSubClass;
	uint8_t  bDeviceProtocol;
	uint8_t  bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t  iManufacturer;
	uint8_t  iProduct;
	uint8_t  iSerialNumber;
	uint8_t  bNumConfigurations;
} __attribute__ ((packed)) usb_device_descriptor_t;

#define USB_DT_DEVICE_SIZE		18

typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bFirstInterface;
	uint8_t  bInterfaceCount;
	uint8_t  bFunctionClass;
	uint8_t  bFunctionSubClass;
	uint8_t  bFunctionProtocol;
	uint8_t  iFunction;
} __attribute__ ((packed)) usb_interface_assoc_descriptor_t;

#define USB_DT_INTERFACE_ASSOCIATION_SIZE	8

typedef struct {
     uint8_t bLength;        
     uint8_t bDescriptorType;    
     uint16_t wTotalLength;      
     uint8_t bNumInterfaces;     
     uint8_t bConfigurationValue;    
     uint8_t iConfiguration;     
     uint8_t bmAttributes;       
     uint8_t MaxPower;       
 } __attribute__ ((packed)) usb_config_descriptor_t;

typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bInterfaceNumber;
	uint8_t  bAlternateSetting;
	uint8_t  bNumEndpoints;
	uint8_t  bInterfaceClass;
	uint8_t  bInterfaceSubClass;
	uint8_t  bInterfaceProtocol;
	uint8_t  iInterface;
} __attribute__ ((packed)) usb_interface_descriptor_t;

#define USB_DT_INTERFACE_SIZE		9

typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bEndpointAddress;
	uint8_t  bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t  bInterval;
} __attribute__ ((packed)) usb_endpoint_descriptor_t;

#define USB_DT_ENDPOINT_SIZE		7

typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint16_t	bcdCDC;
} __attribute__ ((packed)) usb_cdc_header_desc_t;

typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bMasterInterface0;
	uint8_t	bSlaveInterface0;
	/* ... and there could be other slave interfaces */
} __attribute__ ((packed)) usb_cdc_union_desc_t;

typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	iMACAddress;
	uint32_t	bmEthernetStatistics;
	uint16_t	wMaxSegmentSize;
	uint16_t	wNumberMCFilters;
	uint8_t	bNumberPowerFilters;
} __attribute__ ((packed)) usb_cdc_ether_desc_t;

typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint16_t	bcdNcmVersion;
	uint8_t	bmNetworkCapabilities;
} __attribute__ ((packed)) usb_cdc_ncm_desc_t;

/* CONTROL REQUEST SUPPORT */

/*
 * USB directions
 *
 * This bit flag is used in endpoint descriptors' bEndpointAddress field.
 * It's also one of three fields in control requests bRequestType.
 */
#define USB_DIR_OUT			0		/* to device */
#define USB_DIR_IN			0x80		/* to host */

/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK			(0x03 << 5)
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK			0x1f
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03
/* From Wireless USB 1.0 */
#define USB_RECIP_PORT			0x04
#define USB_RECIP_RPIPE		0x05

/*
 * Standard requests, for the bRequest field of a SETUP packet.
 *
 * These are qualified by the bRequestType field, so that for example
 * TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
 * by a GET_STATUS request.
 */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		0x03
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C
#define USB_REQ_SET_SEL			0x30
#define USB_REQ_SET_ISOCH_DELAY		0x31

#define USB_REQ_SET_ENCRYPTION		0x0D	/* Wireless USB */
#define USB_REQ_GET_ENCRYPTION		0x0E
#define USB_REQ_RPIPE_ABORT		0x0E
#define USB_REQ_SET_HANDSHAKE		0x0F
#define USB_REQ_RPIPE_RESET		0x0F
#define USB_REQ_GET_HANDSHAKE		0x10
#define USB_REQ_SET_CONNECTION		0x11
#define USB_REQ_SET_SECURITY_DATA	0x12
#define USB_REQ_GET_SECURITY_DATA	0x13
#define USB_REQ_SET_WUSB_DATA		0x14
#define USB_REQ_LOOPBACK_DATA_WRITE	0x15
#define USB_REQ_LOOPBACK_DATA_READ	0x16
#define USB_REQ_SET_INTERFACE_DS	0x17

/* specific requests for USB Power Delivery */
#define USB_REQ_GET_PARTNER_PDO		20
#define USB_REQ_GET_BATTERY_STATUS	21
#define USB_REQ_SET_PDO			22
#define USB_REQ_GET_VDM			23
#define USB_REQ_SEND_VDM		24

/* The Link Power Management (LPM) ECN defines USB_REQ_TEST_AND_SET command,
 * used by hubs to put ports into a new L1 suspend state, except that it
 * forgot to define its number ...
 */

/*
 * USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, and
 * are read as a bit array returned by USB_REQ_GET_STATUS.  (So there
 * are at most sixteen features of each type.)  Hubs may also support a
 * new USB_REQ_TEST_AND_SET_FEATURE to put ports into L1 suspend.
 */
#define USB_DEVICE_SELF_POWERED		0	/* (read only) */
#define USB_DEVICE_REMOTE_WAKEUP	1	/* dev may initiate wakeup */
#define USB_DEVICE_TEST_MODE		2	/* (wired high speed only) */
#define USB_DEVICE_BATTERY		2	/* (wireless) */
#define USB_DEVICE_B_HNP_ENABLE		3	/* (otg) dev may initiate HNP */
#define USB_DEVICE_WUSB_DEVICE		3	/* (wireless)*/
#define USB_DEVICE_A_HNP_SUPPORT	4	/* (otg) RH port supports HNP */
#define USB_DEVICE_A_ALT_HNP_SUPPORT	5	/* (otg) other RH port does */
#define USB_DEVICE_DEBUG_MODE		6	/* (special devices only) */

/*
 * Test Mode Selectors
 * See USB 2.0 spec Table 9-7
 */
#define	USB_TEST_J		1
#define	USB_TEST_K		2
#define	USB_TEST_SE0_NAK	3
#define	USB_TEST_PACKET		4
#define	USB_TEST_FORCE_ENABLE	5

/* Status Type */
#define USB_STATUS_TYPE_STANDARD	0
#define USB_STATUS_TYPE_PTM		1

/*
 * New Feature Selectors as added by USB 3.0
 * See USB 3.0 spec Table 9-7
 */
#define USB_DEVICE_U1_ENABLE	48	/* dev may initiate U1 transition */
#define USB_DEVICE_U2_ENABLE	49	/* dev may initiate U2 transition */
#define USB_DEVICE_LTM_ENABLE	50	/* dev may send LTM */
#define USB_INTRF_FUNC_SUSPEND	0	/* function suspend */

#define USB_INTR_FUNC_SUSPEND_OPT_MASK	0xFF00
/*
 * Suspend Options, Table 9-8 USB 3.0 spec
 */
#define USB_INTRF_FUNC_SUSPEND_LP	(1 << (8 + 0))
#define USB_INTRF_FUNC_SUSPEND_RW	(1 << (8 + 1))

/*
 * Interface status, Figure 9-5 USB 3.0 spec
 */
#define USB_INTRF_STAT_FUNC_RW_CAP     1
#define USB_INTRF_STAT_FUNC_RW         2

#define USB_ENDPOINT_HALT		0	/* IN/OUT will STALL */

/* Bit array elements as returned by the USB_REQ_GET_STATUS request. */
#define USB_DEV_STAT_U1_ENABLED		2	/* transition into U1 state */
#define USB_DEV_STAT_U2_ENABLED		3	/* transition into U2 state */
#define USB_DEV_STAT_LTM_ENABLED	4	/* Latency tolerance messages */


/*
 * STANDARD DESCRIPTORS ... as returned by GET_DESCRIPTOR, or
 * (rarely) accepted by SET_DESCRIPTOR.
 *
 * Note that all multi-byte values here are encoded in little endian
 * byte order "on the wire".  Within the kernel and when exposed
 * through the Linux-USB APIs, they are not converted to cpu byte
 * order; it is the responsibility of the client code to do this.
 * The single exception is when device and configuration descriptors (but
 * not other descriptors) are read from character devices
 * (i.e. /dev/bus/usb/BBB/DDD);
 * in this case the fields are converted to host endianness by the kernel.
 */

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			0x09
#define USB_DT_DEBUG			0x0a
#define USB_DT_INTERFACE_ASSOCIATION	0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY			0x0c
#define USB_DT_KEY			0x0d
#define USB_DT_ENCRYPTION_TYPE		0x0e
#define USB_DT_BOS			0x0f
#define USB_DT_DEVICE_CAPABILITY	0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP	0x11
#define USB_DT_WIRE_ADAPTER		0x21
#define USB_DT_RPIPE			0x22
#define USB_DT_CS_RADIO_CONTROL		0x23
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE		0x24
/* From the USB 3.0 spec */
#define	USB_DT_SS_ENDPOINT_COMP		0x30
/* From the USB 3.1 spec */
#define	USB_DT_SSP_ISOC_ENDPOINT_COMP	0x31

/* Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define USB_DT_CS_DEVICE		(USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG		(USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING		(USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE		(USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT		(USB_TYPE_CLASS | USB_DT_ENDPOINT)

/*
 * Device and/or Interface Class codes
 * as found in bDeviceClass or bInterfaceClass
 * and defined by www.usb.org documents
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PHYSICAL		5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_CDC_DATA		0x0a
#define USB_CLASS_CSCID			0x0b	/* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d	/* content security */
#define USB_CLASS_VIDEO			0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0
#define USB_CLASS_PERSONAL_HEALTHCARE	0x0f
#define USB_CLASS_AUDIO_VIDEO		0x10
#define USB_CLASS_BILLBOARD		0x11
#define USB_CLASS_USB_TYPE_C_BRIDGE	0x12
#define USB_CLASS_MISC			0xef
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

#define USB_SUBCLASS_VENDOR_SPEC	0xff

/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#define USB_ENDPOINT_MAX_ADJUSTABLE	0x80

#define USB_ENDPOINT_MAXP_MASK	0x07ff
#define USB_EP_MAXP_MULT_SHIFT	11
#define USB_EP_MAXP_MULT_MASK	(3 << USB_EP_MAXP_MULT_SHIFT)
#define USB_EP_MAXP_MULT(m) \
	(((m) & USB_EP_MAXP_MULT_MASK) >> USB_EP_MAXP_MULT_SHIFT)

/* The USB 3.0 spec redefines bits 5:4 of bmAttributes as interrupt ep type. */
#define USB_ENDPOINT_INTRTYPE		0x30
#define USB_ENDPOINT_INTR_PERIODIC	(0 << 4)
#define USB_ENDPOINT_INTR_NOTIFICATION	(1 << 4)

#define USB_ENDPOINT_SYNCTYPE		0x0c
#define USB_ENDPOINT_SYNC_NONE		(0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE	(2 << 2)
#define USB_ENDPOINT_SYNC_SYNC		(3 << 2)

#define USB_ENDPOINT_USAGE_MASK		0x30
#define USB_ENDPOINT_USAGE_DATA		0x00
#define USB_ENDPOINT_USAGE_FEEDBACK	0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FB	0x20

#define USB_CDC_SUBCLASS_ACM			0x02
#define USB_CDC_SUBCLASS_ETHERNET		0x06
#define USB_CDC_SUBCLASS_WHCM			0x08
#define USB_CDC_SUBCLASS_DMM			0x09
#define USB_CDC_SUBCLASS_MDLM			0x0a
#define USB_CDC_SUBCLASS_OBEX			0x0b
#define USB_CDC_SUBCLASS_EEM			0x0c
#define USB_CDC_SUBCLASS_NCM			0x0d
#define USB_CDC_SUBCLASS_MBIM			0x0e
#define USB_CDC_PROTO_NONE			0
#define USB_CDC_ACM_PROTO_AT_V25TER		1
#define USB_CDC_ACM_PROTO_AT_PCCA101		2
#define USB_CDC_ACM_PROTO_AT_PCCA101_WAKE	3
#define USB_CDC_ACM_PROTO_AT_GSM		4
#define USB_CDC_ACM_PROTO_AT_3G			5
#define USB_CDC_ACM_PROTO_AT_CDMA		6
#define USB_CDC_ACM_PROTO_VENDOR		0xff
#define USB_CDC_PROTO_EEM			7
#define USB_CDC_NCM_PROTO_NTB			1
#define USB_CDC_MBIM_PROTO_NTB			2
/*-------------------------------------------------------------------------*/
/*
 * Class-Specific descriptors ... there are a couple dozen of them
 */
#define USB_CDC_HEADER_TYPE		0x00	/* header_desc */
#define USB_CDC_CALL_MANAGEMENT_TYPE	0x01	/* call_mgmt_descriptor */
#define USB_CDC_ACM_TYPE		0x02	/* acm_descriptor */
#define USB_CDC_UNION_TYPE		0x06	/* union_desc */
#define USB_CDC_COUNTRY_TYPE		0x07
#define USB_CDC_NETWORK_TERMINAL_TYPE	0x0a	/* network_terminal_desc */
#define USB_CDC_ETHERNET_TYPE		0x0f	/* ether_desc */
#define USB_CDC_WHCM_TYPE		0x11
#define USB_CDC_MDLM_TYPE		0x12	/* mdlm_desc */
#define USB_CDC_MDLM_DETAIL_TYPE	0x13	/* mdlm_detail_desc */
#define USB_CDC_DMM_TYPE		0x14
#define USB_CDC_OBEX_TYPE		0x15
#define USB_CDC_NCM_TYPE		0x1a
#define USB_CDC_MBIM_TYPE		0x1b
#define USB_CDC_MBIM_EXTENDED_TYPE	0x1c
/* "Call Management Descriptor" from CDC spec  5.2.3.2 */
typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bmCapabilities;
#define USB_CDC_CALL_MGMT_CAP_CALL_MGMT		0x01
#define USB_CDC_CALL_MGMT_CAP_DATA_INTF		0x02
	uint8_t	bDataInterface;
} __attribute__ ((packed)) usb_cdc_call_mgmt_descriptor_t;
/* "Abstract Control Management Descriptor" from CDC spec  5.2.3.3 */
typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bmCapabilities;
} __attribute__ ((packed)) usb_cdc_acm_descriptor_t;
/* capabilities from 5.2.3.3 */
#define USB_CDC_COMM_FEATURE	0x01
#define USB_CDC_CAP_LINE	0x02
#define USB_CDC_CAP_BRK		0x04
#define USB_CDC_CAP_NOTIFY	0x08
/* "Country Selection Functional Descriptor" from CDC spec 5.2.3.9 */
typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	iCountryCodeRelDate;
	uint16_t	wCountyCode0;
	/* ... and there can be a lot of country codes */
} __attribute__ ((packed)) usb_cdc_country_functional_desc_t;
/* "Network Channel Terminal Functional Descriptor" from CDC spec 5.2.3.11 */
typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bEntityId;
	uint8_t	iName;
	uint8_t	bChannelIndex;
	uint8_t	bPhysicalInterface;
} __attribute__ ((packed)) usb_cdc_network_terminal_desc_t;
/* "Telephone Control Model Functional Descriptor" from CDC WMC spec 6.3..3 */
typedef struct {
	uint8_t	bFunctionLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubtype;
	uint16_t	bcdVersion;
	uint16_t	wMaxCommand;
} __attribute__ ((packed)) usb_cdc_dmm_desc_t;
/* "MDLM Functional Descriptor" from CDC WMC spec 6.7.2.3 */
typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint16_t	bcdVersion;
	uint8_t	bGUID[16];
} __attribute__ ((packed)) usb_cdc_mdlm_desc_t;
/* "MDLM Detail Functional Descriptor" from CDC WMC spec 6.7.2.4 */
typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	/* type is associated with mdlm_desc.bGUID */
	uint8_t	bGuidDescriptorType;
	uint8_t	bDetailData[0];
} __attribute__ ((packed)) usb_cdc_mdlm_detail_desc_t;
/* "OBEX Control Model Functional Descriptor" */
typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint16_t	bcdVersion;
} __attribute__ ((packed)) usb_cdc_obex_desc_t;
/* "MBIM Control Model Functional Descriptor" */
typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint16_t	bcdMBIMVersion;
	uint16_t  wMaxControlMessage;
	uint8_t    bNumberFilters;
	uint8_t    bMaxFilterSize;
	uint16_t  wMaxSegmentSize;
	uint8_t    bmNetworkCapabilities;
} __attribute__ ((packed)) usb_cdc_mbim_desc_t;
/* "MBIM Extended Functional Descriptor" from CDC MBIM spec 1.0 errata-1 */
typedef struct {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint16_t	bcdMBIMExtendedVersion;
	uint8_t	bMaxOutstandingCommandMessages;
	uint16_t	wMTU;
} __attribute__ ((packed)) usb_cdc_mbim_extended_desc_t;
/*-------------------------------------------------------------------------*/
/*
 * Class-Specific Control Requests (6.2)
 *
 * section 3.6.2.1 table 4 has the ACM profile, for modems.
 * section 3.8.2 table 10 has the ethernet profile.
 *
 * Microsoft's RNDIS stack for Ethernet is a vendor-specific CDC ACM variant,
 * heavily dependent on the encapsulated (proprietary) command mechanism.
 */
#define USB_CDC_SEND_ENCAPSULATED_COMMAND	0x00
#define USB_CDC_GET_ENCAPSULATED_RESPONSE	0x01
#define USB_CDC_RESET_FUNCTION			0x05
#define USB_CDC_REQ_SET_LINE_CODING		0x20
#define USB_CDC_REQ_GET_LINE_CODING		0x21
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE	0x22
#define USB_CDC_REQ_SEND_BREAK			0x23
#define USB_CDC_SET_ETHERNET_MULTICAST_FILTERS	0x40
#define USB_CDC_SET_ETHERNET_PM_PATTERN_FILTER	0x41
#define USB_CDC_GET_ETHERNET_PM_PATTERN_FILTER	0x42
#define USB_CDC_SET_ETHERNET_PACKET_FILTER	0x43
#define USB_CDC_GET_ETHERNET_STATISTIC		0x44
#define USB_CDC_GET_NTB_PARAMETERS		0x80
#define USB_CDC_GET_NET_ADDRESS			0x81
#define USB_CDC_SET_NET_ADDRESS			0x82
#define USB_CDC_GET_NTB_FORMAT			0x83
#define USB_CDC_SET_NTB_FORMAT			0x84
#define USB_CDC_GET_NTB_INPUT_SIZE		0x85
#define USB_CDC_SET_NTB_INPUT_SIZE		0x86
#define USB_CDC_GET_MAX_DATAGRAM_SIZE		0x87
#define USB_CDC_SET_MAX_DATAGRAM_SIZE		0x88
#define USB_CDC_GET_CRC_MODE			0x89
#define USB_CDC_SET_CRC_MODE			0x8a
/* Line Coding Structure from CDC spec 6.2.13 */
typedef struct {
	uint32_t	dwDTERate;
	uint8_t	bCharFormat;
#define USB_CDC_1_STOP_BITS			0
#define USB_CDC_1_5_STOP_BITS			1
#define USB_CDC_2_STOP_BITS			2
	uint8_t	bParityType;
#define USB_CDC_NO_PARITY			0
#define USB_CDC_ODD_PARITY			1
#define USB_CDC_EVEN_PARITY			2
#define USB_CDC_MARK_PARITY			3
#define USB_CDC_SPACE_PARITY			4
	uint8_t	bDataBits;
} __attribute__ ((packed)) usb_cdc_line_coding_t;
/* table 62; bits in multicast filter */
#define	USB_CDC_PACKET_TYPE_PROMISCUOUS		(1 << 0)
#define	USB_CDC_PACKET_TYPE_ALL_MULTICAST	(1 << 1) /* no filter */
#define	USB_CDC_PACKET_TYPE_DIRECTED		(1 << 2)
#define	USB_CDC_PACKET_TYPE_BROADCAST		(1 << 3)
#define	USB_CDC_PACKET_TYPE_MULTICAST		(1 << 4) /* filtered */
/*-------------------------------------------------------------------------*/
/*
 * Class-Specific Notifications (6.3) sent by interrupt transfers
 *
 * section 3.8.2 table 11 of the CDC spec lists Ethernet notifications
 * section 3.6.2.1 table 5 specifies ACM notifications, accepted by RNDIS
 * RNDIS also defines its own bit-incompatible notifications
 */
#define USB_CDC_NOTIFY_NETWORK_CONNECTION	0x00
#define USB_CDC_NOTIFY_RESPONSE_AVAILABLE	0x01
#define USB_CDC_NOTIFY_SERIAL_STATE		0x20
#define USB_CDC_NOTIFY_SPEED_CHANGE		0x2a
typedef struct {
	uint8_t	bmRequestType;
	uint8_t	bNotificationType;
	uint16_t	wValue;
	uint16_t	wIndex;
	uint16_t	wLength;
} __attribute__ ((packed)) usb_cdc_notification_t;
typedef struct {
	uint32_t	DLBitRRate;	/* contains the downlink bit rate (IN pipe) */
	uint32_t	ULBitRate;	/* contains the uplink bit rate (OUT pipe) */
} __attribute__ ((packed)) usb_cdc_speed_change_t;

typedef struct {
	usb_cdc_notification_t header;
	usb_cdc_speed_change_t speeds;
} __attribute__ ((packed)) usb_cdc_notify_speed_t;

/*-------------------------------------------------------------------------*/
/*
 * Class Specific structures and constants
 *
 * CDC NCM NTB parameters structure, CDC NCM subclass 6.2.1
 *
 */
typedef struct {
	uint16_t	wLength;
	uint16_t	bmNtbFormatsSupported;
	uint32_t	dwNtbInMaxSize;
	uint16_t	wNdpInDivisor;
	uint16_t	wNdpInPayloadRemainder;
	uint16_t	wNdpInAlignment;
	uint16_t	wPadding1;
	uint32_t	dwNtbOutMaxSize;
	uint16_t	wNdpOutDivisor;
	uint16_t	wNdpOutPayloadRemainder;
	uint16_t	wNdpOutAlignment;
	uint16_t	wNtbOutMaxDatagrams;
} __attribute__ ((packed)) usb_cdc_ncm_ntb_parameters_t;
/*
 * CDC NCM transfer headers, CDC NCM subclass 3.2
 */
#define USB_CDC_NCM_NTH16_SIGN		0x484D434E /* NCMH */
#define USB_CDC_NCM_NTH32_SIGN		0x686D636E /* ncmh */
typedef struct {
	uint32_t	dwSignature;
	uint16_t	wHeaderLength;
	uint16_t	wSequence;
	uint16_t	wBlockLength;
	uint16_t	wNdpIndex;
} __attribute__ ((packed)) usb_cdc_ncm_nth16_t;
typedef struct {
	uint32_t	dwSignature;
	uint16_t	wHeaderLength;
	uint16_t	wSequence;
	uint32_t	dwBlockLength;
	uint32_t	dwNdpIndex;
} __attribute__ ((packed)) usb_cdc_ncm_nth32_t;
/*
 * CDC NCM datagram pointers, CDC NCM subclass 3.3
 */
#define USB_CDC_NCM_NDP16_CRC_SIGN	0x314D434E /* NCM1 */
#define USB_CDC_NCM_NDP16_NOCRC_SIGN	0x304D434E /* NCM0 */
#define USB_CDC_NCM_NDP32_CRC_SIGN	0x316D636E /* ncm1 */
#define USB_CDC_NCM_NDP32_NOCRC_SIGN	0x306D636E /* ncm0 */
#define USB_CDC_MBIM_NDP16_IPS_SIGN     0x00535049 /* IPS<sessionID> : IPS0 for now */
#define USB_CDC_MBIM_NDP32_IPS_SIGN     0x00737069 /* ips<sessionID> : ips0 for now */
#define USB_CDC_MBIM_NDP16_DSS_SIGN     0x00535344 /* DSS<sessionID> */
#define USB_CDC_MBIM_NDP32_DSS_SIGN     0x00737364 /* dss<sessionID> */
/* 16-bit NCM Datagram Pointer Entry */
typedef struct {
	uint16_t	wDatagramIndex;
	uint16_t	wDatagramLength;
} __attribute__((__packed__)) usb_cdc_ncm_dpe16_t;
/* 16-bit NCM Datagram Pointer Table */
typedef struct {
	uint32_t	dwSignature;
	uint16_t	wLength;
	uint16_t	wNextNdpIndex;
	usb_cdc_ncm_dpe16_t dpe16[0];
} __attribute__ ((packed)) usb_cdc_ncm_ndp16_t;
/* 32-bit NCM Datagram Pointer Entry */
typedef struct {
	uint32_t	dwDatagramIndex;
	uint32_t	dwDatagramLength;
} __attribute__((__packed__)) usb_cdc_ncm_dpe32_t;
/* 32-bit NCM Datagram Pointer Table */
typedef struct {
	uint32_t	dwSignature;
	uint16_t	wLength;
	uint16_t	wReserved6;
	uint32_t	dwNextNdpIndex;
	uint32_t	dwReserved12;
	usb_cdc_ncm_dpe32_t dpe32[0];
} __attribute__ ((packed)) usb_cdc_ncm_ndp32_t;
/* CDC NCM subclass 3.2.1 and 3.2.2 */
#define USB_CDC_NCM_NDP16_INDEX_MIN			0x000C
#define USB_CDC_NCM_NDP32_INDEX_MIN			0x0010
/* CDC NCM subclass 3.3.3 Datagram Formatting */
#define USB_CDC_NCM_DATAGRAM_FORMAT_CRC			0x30
#define USB_CDC_NCM_DATAGRAM_FORMAT_NOCRC		0X31
/* CDC NCM subclass 4.2 NCM Communications Interface Protocol Code */
#define USB_CDC_NCM_PROTO_CODE_NO_ENCAP_COMMANDS	0x00
#define USB_CDC_NCM_PROTO_CODE_EXTERN_PROTO		0xFE
/* CDC NCM subclass 5.2.1 NCM Functional Descriptor, bmNetworkCapabilities */
#define USB_CDC_NCM_NCAP_ETH_FILTER			(1 << 0)
#define USB_CDC_NCM_NCAP_NET_ADDRESS			(1 << 1)
#define USB_CDC_NCM_NCAP_ENCAP_COMMAND			(1 << 2)
#define USB_CDC_NCM_NCAP_MAX_DATAGRAM_SIZE		(1 << 3)
#define USB_CDC_NCM_NCAP_CRC_MODE			(1 << 4)
#define	USB_CDC_NCM_NCAP_NTB_INPUT_SIZE			(1 << 5)
/* CDC NCM subclass Table 6-3: NTB Parameter Structure */
#define USB_CDC_NCM_NTB16_SUPPORTED			(1 << 0)
#define USB_CDC_NCM_NTB32_SUPPORTED			(1 << 1)
/* CDC NCM subclass Table 6-3: NTB Parameter Structure */
#define USB_CDC_NCM_NDP_ALIGN_MIN_SIZE			0x04
#define USB_CDC_NCM_NTB_MAX_LENGTH			0x1C
/* CDC NCM subclass 6.2.5 SetNtbFormat */
#define USB_CDC_NCM_NTB16_FORMAT			0x00
#define USB_CDC_NCM_NTB32_FORMAT			0x01
/* CDC NCM subclass 6.2.7 SetNtbInputSize */
#define USB_CDC_NCM_NTB_MIN_IN_SIZE			2048
#define USB_CDC_NCM_NTB_MIN_OUT_SIZE			2048
#define USB_CDC_NCM_NTB_MAX_IN_SIZE			3200
#define USB_CDC_NCM_NTB_MAX_OUT_SIZE			3200
/* NTB Input Size Structure */
typedef struct {
	uint32_t	dwNtbInMaxSize;
	uint16_t	wNtbInMaxDatagrams;
	uint16_t	wReserved;
} __attribute__ ((packed)) usb_cdc_ncm_ndp_input_size_t;
/* CDC NCM subclass 6.2.11 SetCrcMode */
#define USB_CDC_NCM_CRC_NOT_APPENDED			0x00
#define USB_CDC_NCM_CRC_APPENDED			0x01