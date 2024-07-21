#pragma once

#include "desc_value.h"

#include <stdint.h>

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
  struct {
    uint8_t recipient : 5;
	  uint8_t type : 2;
    uint8_t direction : 1;
  } __attribute__ ((packed)) bmRequestType;
  uint8_t  bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
} __attribute__ ((packed)) usb_control_request_t;

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

typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bEndpointAddress;
	uint8_t  bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t  bInterval;
} __attribute__ ((packed)) usb_endpoint_descriptor_t;

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