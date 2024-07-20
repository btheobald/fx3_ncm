#pragma once

// USB direction
#define USB_DIR_OUT					0x00	// to device
#define USB_DIR_IN					0x80	// to host

#define USB_DATA_HOST_TO_DEVICE     0x00
#define USB_DATA_DEVICE_TO_HOST     0x01

// USB types, the second of three bRequestType fields
#define USB_TYPE_STANDARD			0x00
#define USB_TYPE_CLASS				0x01
#define USB_TYPE_VENDOR				0x02
#define USB_TYPE_RESERVED			0x03

// USB reciepient
#define USB_RECIP_DEVICE			0x00
#define USB_RECIP_INTERFACE			0x01
#define USB_RECIP_ENDPOINT			0x02
#define USB_RECIP_OTHER				0x03

// Standard requests, for the bRequest field of a SETUP packet.
#define USB_REQ_GET_STATUS			0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE			0x03
#define USB_REQ_SET_ADDRESS			0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME			0x0C
#define USB_REQ_SET_SEL				0x30
#define USB_REQ_SET_ISOCH_DELAY		0x31

// USB Power Delivery
#define USB_REQ_GET_PARTNER_PDO		20
#define USB_REQ_GET_BATTERY_STATUS	21
#define USB_REQ_SET_PDO				22
#define USB_REQ_GET_VDM				23
#define USB_REQ_SEND_VDM			24

// Feature Flags
#define USB_DEVICE_SELF_POWERED		0
#define USB_DEVICE_REMOTE_WAKEUP	1
#define USB_DEVICE_TEST_MODE		2

// Status Type
#define USB_STATUS_TYPE_STANDARD	0

// USB 3.0
#define USB_DEVICE_U1_ENABLE		48	// Devive may initiate U1 transition
#define USB_DEVICE_U2_ENABLE		49	// Device may initiate U2 transition
#define USB_DEVICE_LTM_ENABLE		50	// Device may send LTM
#define USB_INTRF_FUNC_SUSPEND		0	// Function suspend

// Interface status
#define USB_INTRF_STAT_FUNC_RW_CAP  1
#define USB_INTRF_STAT_FUNC_RW      2

#define USB_ENDPOINT_HALT			0	// IN/OUT will STALL

// Bit array elements as returned by the USB_REQ_GET_STATUS request.
#define USB_DEV_STAT_U1_ENABLED		2	// transition into U1 state
#define USB_DEV_STAT_U2_ENABLED		3	// transition into U2 state
#define USB_DEV_STAT_LTM_ENABLED	4	// Latency tolerance messages

// Descriptor Types
#define USB_DT_DEVICE				0x01
#define USB_DT_CONFIG				0x02
#define USB_DT_STRING				0x03
#define USB_DT_INTERFACE			0x04
#define USB_DT_ENDPOINT				0x05
#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
#define USB_DT_OTG					0x09
#define USB_DT_DEBUG				0x0a
#define USB_DT_INTERFACE_ASSOC		0x0b
#define	USB_DT_SS_EP_COMP			0x30
#define	USB_DT_SSP_ISOC_EP_COMP		0x31

// Class Specific Descriptors
#define USB_DT_CS_DEVICE			(USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG			(USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING			(USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE			(USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT			(USB_TYPE_CLASS | USB_DT_ENDPOINT)

// Device / Interface Class Codes
#define USB_CLASS_PER_INTERFACE		0x00
#define USB_CLASS_COMM				0x02
#define USB_CLASS_CDC_DATA			0x0a
#define USB_CLASS_VENDOR_SPEC		0xff

// Endpoints
#define USB_ENDPOINT_NUMBER_MASK	0x0f
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK	0x03
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#define USB_ENDPOINT_MAX_ADJUSTABLE	0x80

#define USB_ENDPOINT_MAXP_MASK		0x07ff
#define USB_EP_MAXP_MULT_SHIFT		11
#define USB_EP_MAXP_MULT_MASK		(3 << USB_EP_MAXP_MULT_SHIFT)
#define USB_EP_MAXP_MULT(m) 		(((m) & USB_EP_MAXP_MULT_MASK) >> USB_EP_MAXP_MULT_SHIFT)

#define USB_ENDPOINT_INTRTYPE		0x30
#define USB_ENDPOINT_INTR_PERIODIC	(0 << 4)
#define USB_ENDPOINT_INTR_NOTIF		(1 << 4)

#define USB_ENDPOINT_SYNCTYPE		0x0c
#define USB_ENDPOINT_SYNC_NONE		(0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE	(2 << 2)
#define USB_ENDPOINT_SYNC_SYNC		(3 << 2)

#define USB_ENDPOINT_USAGE_MASK		0x30
#define USB_ENDPOINT_USAGE_DATA		0x00
#define USB_ENDPOINT_USAGE_FEEDBACK	0x10
#define USB_ENDPOINT_USAGE_IMPLI_FB	0x20

#define USB_CDC_SUBCLASS_NCM		0x0d
#define USB_CDC_PROTO_NONE			0
#define USB_CDC_NCM_PROTO_NTB		1

// Class-Specific descriptors
#define USB_CDC_HEADER_TYPE			0x00
#define USB_CDC_UNION_TYPE			0x06
#define USB_CDC_ETHERNET_TYPE		0x0f
#define USB_CDC_NCM_TYPE			0x1a

// CDC Packet Types
#define	USB_CDC_PACKET_PROMISCUOUS	(1 << 0)
#define	USB_CDC_PACKET_ALL_MCAST	(1 << 1)
#define	USB_CDC_PACKET_DIRECTED		(1 << 2)
#define	USB_CDC_PACKET_BROADCAST	(1 << 3)
#define	USB_CDC_PACKET_MULTICAST	(1 << 4)