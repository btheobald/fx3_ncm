#include "net.h"
#include "ncm.h"
#include "struct.h"
#include "tx.h"
#include "rx.h"
#include "usb.h"
#include "desc_types.h"
#include "desc_value.h"
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#define MOD_NET "NET"

bool net_can_tx( uint16_t size ) {
    USB_LOG(MOD_NET, "net_can_tx\r\n");
    if( tx_datagram_fits_user_ntb( size ) || tx_accept_user_ntb()) {
        USB_LOG(MOD_NET, "\tTX Allowed\r\n");
        return true;
    }
    USB_LOG(MOD_NET, "\tTX Blocked\r\n");
    tx_try_start();
    return false;
}

void net_tx( void * datagram, uint16_t arg ) {
    USB_LOG(MOD_NET, "net_tx\r\n");
    if( ncm_if.tx_user_ntb == NULL ) {
        return;
    }
    ntb_def_t * ntb = (ntb_def_t *)(ncm_if.tx_user_ntb);
    uint16_t size = net_tx_cb( &ntb->data[ntb->nth.wBlockLength], datagram, arg );
    ntb->ndp_datagram[ ncm_if.tx_user_ntb_datagram_index ].wDatagramIndex = ntb->nth.wBlockLength;
    ntb->ndp_datagram[ ncm_if.tx_user_ntb_datagram_index ].wDatagramLength = size;
    ncm_if.tx_user_ntb_datagram_index++;
    ntb->nth.wBlockLength += size + ALIGN_OFFSET( size );
    if( ntb->nth.wBlockLength > NCM_NTB_MAX_SIZE ) {
        USB_LOG(MOD_NET, "!!! CRITICAL BUFFER OVERFLOW !!!\r\n");
        return;
    }
    tx_try_start();
}

void net_rx_renew( void ) {
    USB_LOG(MOD_NET, "net_rx_renew\r\n");
    rx_update_user_ntb();
    rx_try_start();
}

void net_init( void ) {
    USB_LOG(MOD_NET, "net_init\r\n");
    // Zero out NCM structure
    memset(&ncm_if, 0, sizeof(ncm_if));
    // All NTBs start out as free
    for( int n = 0; n < NCM_NTB_N; n++ ) {
        push_ntb(&ncm_if.tx_free_ntb, &ncm_if.tx_ntb[n]); 
        push_ntb(&ncm_if.rx_free_ntb, &ncm_if.rx_ntb[n]);
    }
}

bool net_usb_transfer( uint8_t ep_addr, uint16_t bytes ) {
    USB_LOG(MOD_NET, "net_usb_transfer\r\n");
    switch( ep_addr ) {
        case NCM_EP_DATA_OUT:
            if( rx_validate_datagram( ncm_if.rx_usbd_ntb, bytes ) ) {
                USB_LOG(MOD_NET, "\tGot Valid Datagram from USB\r\n");
                push_ntb( &ncm_if.rx_wait_ntb, ncm_if.rx_usbd_ntb ); 
            } else {
                USB_LOG(MOD_NET, "\tGot Invalid Datagram from USB\r\n");
                ncm_if.rx_usbd_ntb = NULL;
            }
            net_rx_renew();
            break;
        case NCM_EP_DATA_IN:
            USB_LOG(MOD_NET, "\tSending NTB to USB\r\n");
            push_ntb( &ncm_if.rx_free_ntb, ncm_if.rx_usbd_ntb );
            if( !tx_insert_zlp( bytes ) ) {
                tx_try_start();
            }
            break;
        case NCM_EP_NOTIFY:
            USB_LOG(MOD_NET, "\tSending Notification\r\n");
            tx_notification( true );
    }
    return true;
}

bool net_usb_control_transfer( usb_control_request_t * request ) {
    USB_LOG(MOD_NET, "net_usb_control_transfer\r\n");
    if( request->bmRequestType.type == USB_TYPE_STANDARD ) {
        if( request->bRequest == USB_REQ_GET_INTERFACE) {
            USB_LOG(MOD_NET, "\tUSB Request Interface State\r\n");
            usb_transfer_ep0 ( request, &(ncm_if.interface_active), 1 );
        } else if( request->bRequest == USB_REQ_SET_INTERFACE ) {
            ncm_if.interface_active = request->wValue;
            if( ncm_if.interface_active ) {
                rx_update_user_ntb();
                tx_notification( false );
                USB_LOG(MOD_NET, "\tUSB Set Interface Active\r\n");
            } else {
                USB_LOG(MOD_NET, "\tUSB Set Interface Inactive\r\n");
            }
            usb_control_status( request );
        } else {
            return false;
        }
    } else if( request->bmRequestType.type == USB_TYPE_CLASS ) {
        if( request->bRequest == NCM_GET_NTB_PARAMETERS ) {
            usb_transfer_ep0 ( request, &ntb_parameters, sizeof(ntb_parameters_t) );
            USB_LOG(MOD_NET, "\tUSB Get NTB Parameters\r\n");
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}