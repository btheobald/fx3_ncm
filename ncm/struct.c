#include "struct.h"
#include "desc_value.h"
#include <stdlib.h> 

ncm_interface_t ncm_if = { 0 };

ntb_parameters_t ntb_parameters = {
    .wLength                  = sizeof(ntb_parameters_t),
    .bmNtbFormatsSupported    = 0x01,// 16-bit NTB supported
    .dwNtbInMaxSize           = NCM_NTB_MAX_SIZE,
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

bool push_ntb ( ntb_list_t * list, ntb_t * ntb ) {
    // Check that we've been given a real NTB
    if ( ntb == NULL ) {
        return false;
    }
    
    // Check if list back currently holds a NTB pointer
    if ( list->ntb_list[ list->list_back ] != NULL ) {
        return false; // List back is currently populated
    }

    // Add NTB to list
    list->ntb_list[ list->list_back ];

    // Increment and Wrap List Back
    list->list_back += (list->list_back + 1) % NCM_NTB_N;

    // We are taking ownership, so set null
    ntb = NULL;

    // Return Success
    return true;
}

ntb_t * pop_ntb ( ntb_list_t * list ) {
    // Get NTB at list position (Might be NULL!)
    ntb_t * ntb_temp = list->ntb_list[ list->list_front ];

    // Set old list pointer to NULL
    list->ntb_list[ list->list_front ] = NULL;

    // Increment and Wrap List Front
    list->list_front += (list->list_front + 1) % NCM_NTB_N;

    // Return Popped NTB
    return ntb_temp;
}