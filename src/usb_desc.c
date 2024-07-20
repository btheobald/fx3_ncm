#include "usb_desc.h" 

#define NCM_STATUS_BYTECOUNT		16	/* 8 byte header + data */

usb_device_descriptor_t const usb_device_desc __attribute__ ((aligned (32))) = {
	.bLength            =   sizeof(usb_device_descriptor_t),
	.bDescriptorType    =   USB_DT_DEVICE,
	.bcdUSB             =   0x200,
	.bDeviceClass       =   USB_CLASS_COMM,
	.bDeviceSubClass    =   USB_CDC_SUBCLASS_NCM,
	.bDeviceProtocol    =   USB_CDC_PROTO_NONE,
	.bMaxPacketSize0    =   64,
	.idVendor           =   0xCAFE,
	.idProduct          =   0xEEEE,
	.bcdDevice          =   0x0101,
	.iManufacturer      =   1,
	.iProduct           =   2,
	.iSerialNumber      =   3,
	.bNumConfigurations =   1
};

usb_desc_t usb_config_desc __attribute__ ((aligned (32))) = {
    .config_desc = {
        .bLength            =   sizeof(usb_config_descriptor_t),        
        .bDescriptorType    =   USB_DT_CONFIG,    
        .wTotalLength       =   sizeof(usb_desc_t),      
        .bNumInterfaces     =   2,     
        .bConfigurationValue =  1,    
        .iConfiguration     =   0,     
        .bmAttributes       =   0x80,       
        .MaxPower           =   500/2
    },
    .ncm_iad_desc = {
        .bLength            =	sizeof(usb_interface_assoc_descriptor_t),
        .bDescriptorType    =	USB_DT_INTERFACE_ASSOC,
        .bFirstInterface    =	0,
        .bInterfaceCount    =	2,	/* control + data */
        .bFunctionClass     =	USB_CLASS_COMM,
        .bFunctionSubClass  =	USB_CDC_SUBCLASS_NCM,
        .bFunctionProtocol  =	USB_CDC_PROTO_NONE,
        /* .iFunction =		DYNAMIC */
    },
    .ncm_control_intf = {
        .bLength            =	sizeof(usb_interface_descriptor_t),
        .bDescriptorType    =	USB_DT_INTERFACE,
        .bInterfaceNumber   =   0,
        .bNumEndpoints      =	1,
        .bInterfaceClass    =	USB_CLASS_COMM,
        .bInterfaceSubClass =	USB_CDC_SUBCLASS_NCM,
        .bInterfaceProtocol =	USB_CDC_PROTO_NONE,
        .iInterface         =   STR_ID_INTF,
    },
    .ncm_header_desc = {
        .bLength            =	sizeof(usb_cdc_header_desc_t),
        .bDescriptorType    =	USB_DT_CS_INTERFACE,
        .bDescriptorSubType =	USB_CDC_HEADER_TYPE,
        .bcdCDC             =	0x0110,
    },
    .ncm_union_desc = {
        .bLength            =	sizeof(usb_cdc_union_desc_t),
        .bDescriptorType    =	USB_DT_CS_INTERFACE,
        .bDescriptorSubType =	USB_CDC_UNION_TYPE,
        .bMasterInterface0  =	0, // Control/* .iInterface = DYNAMIC */ Interface
        .bSlaveInterface0   =	1  // Data Interface
    },
    .ecm_desc = {
        .bLength            =	sizeof(usb_cdc_ether_desc_t),
        .bDescriptorType    =	USB_DT_CS_INTERFACE,
        .bDescriptorSubType =	USB_CDC_ETHERNET_TYPE,
        .iMACAddress        =   STR_ID_MACA,
        .bmEthernetStatistics =	0, /* no statistics */
        .wMaxSegmentSize    =	1514,
        .wNumberMCFilters   =	0,
        .bNumberPowerFilters =	0,
    },
    .ncm_desc = {
        .bLength            =	sizeof(usb_cdc_ncm_desc_t),
        .bDescriptorType    =	USB_DT_CS_INTERFACE,
        .bDescriptorSubType =	USB_CDC_NCM_TYPE,
        .bcdNcmVersion      =	0x0100,
        //.bmNetworkCapabilities = (USB_CDC_NCM_NCAP_ETH_FILTER | USB_CDC_NCM_NCAP_CRC_MODE),
    },
    .ncm_notify_ep_desc = {
        .bLength            =	sizeof(usb_endpoint_descriptor_t),
        .bDescriptorType    =	USB_DT_ENDPOINT,
        .bEndpointAddress   =	CY_FX_EP_NOTIFIER,
        .bmAttributes       =	USB_ENDPOINT_XFER_INT,
        .wMaxPacketSize     =	NCM_STATUS_BYTECOUNT,
        .bInterval          =	9,
    },
    .ncm_data_nop_intf = {
        .bLength            =	sizeof(usb_interface_descriptor_t),
	    .bDescriptorType    =	USB_DT_INTERFACE,
	    .bInterfaceNumber   =	1,
	    .bAlternateSetting  =	0,
	    .bNumEndpoints      =	0,
	    .bInterfaceClass    =	USB_CLASS_CDC_DATA,
	    .bInterfaceSubClass =	0,
	    .bInterfaceProtocol =	USB_CDC_NCM_PROTO_NTB,
	    .iInterface         =   STR_ID_INTF,
    },
    .ncm_data_intf = {
        .bLength            =	sizeof(usb_interface_descriptor_t),
        .bDescriptorType    =	USB_DT_INTERFACE,
        .bInterfaceNumber   =	1,
        .bAlternateSetting  =	1,
        .bNumEndpoints      =	2,
        .bInterfaceClass    =	USB_CLASS_CDC_DATA,
        .bInterfaceSubClass =	0,
        .bInterfaceProtocol =	USB_CDC_NCM_PROTO_NTB,
        .iInterface         =   STR_ID_INTF,
    },
    .ncm_in_ep_desc = {
        .bLength            =	sizeof(usb_endpoint_descriptor_t),
        .bDescriptorType    =	USB_DT_ENDPOINT,
        .bEndpointAddress   =	CY_FX_EP_CONSUMER,
        .bmAttributes       =	USB_ENDPOINT_XFER_BULK,
        .wMaxPacketSize     =	512,
    },
    .ncm_out_ep_desc = {
        .bLength            =	sizeof(usb_endpoint_descriptor_t),
        .bDescriptorType    =	USB_DT_ENDPOINT,
        .bEndpointAddress   =	CY_FX_EP_PRODUCER,
        .bmAttributes       =	USB_ENDPOINT_XFER_BULK,
        .wMaxPacketSize     =	512,
    }
};

const usb_string_descriptor_t str_desc_langid = {
    .bLength = sizeof(STR_LANG),
    .bDescriptorType = USB_DT_STRING,
    .bString = { STR_LANG }
};

const usb_string_descriptor_t str_desc_manufacturer = {
    .bLength = sizeof(STR_MANF),
    .bDescriptorType = USB_DT_STRING,
    .bString = STR_MANF
};

const usb_string_descriptor_t str_desc_product = {
    .bLength = sizeof(STR_PROD),
    .bDescriptorType = USB_DT_STRING,
    .bString = STR_PROD
};

const usb_string_descriptor_t str_desc_interface = {
    .bLength = sizeof(STR_INTF),
    .bDescriptorType = USB_DT_STRING,
    .bString = STR_INTF
};

const usb_string_descriptor_t str_desc_serial = {
    .bLength = sizeof(STR_MACA),
    .bDescriptorType = USB_DT_STRING,
    .bString = STR_MACA
};