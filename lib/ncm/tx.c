#include "tx.h"
#include "ncm.h"
#include "usb.h"
#include "struct.h"
#include <stdlib.h> 
#include <string.h>

#include "debug.h"
#define MOD_TX "TX"

void tx_notification( bool force_next ) {
  USB_LOG(MOD_TX, "tx_notification\r\n");
  if( !force_next && (ncm_if.notification_state != TX_NOTIFY_STATE_WAITING) ) {
    return;
  }

  if (ncm_if.notification_state == TX_NOTIFY_STATE_WAITING) {
    USB_LOG(MOD_TX, "\tNotify Speed, Notify Connected\r\n");
    ncm_if.notification_state = TX_NOTIFY_STATE_SPEED_CHANGE;
    usb_transfer_ep ( NCM_EP_NOTIFY, &notify_speed, sizeof(notify_speed) );
    usb_transfer_ep ( NCM_EP_NOTIFY, &notify_connection, sizeof(notify_connection) );
    ncm_if.notification_state = TX_NOTIFY_STATE_CONNECTED;
  } else if (ncm_if.notification_state == TX_NOTIFY_STATE_CONNECTED) {
    USB_LOG(MOD_TX, "\tState: Waiting\r\n");
    ncm_if.notification_state = TX_NOTIFY_STATE_WAITING;
  }
}

bool tx_insert_zlp( uint16_t size ) {
  USB_LOG(MOD_TX, "tx_insert_zlp\r\n");
  if ( size == 0 || size % NCM_EP_SIZE != 0 ) {
    return false;
  }
  USB_LOG(MOD_TX, "\tTransmit ZLP!\r\n");
  usb_transfer_ep ( NCM_EP_DATA_IN, NULL, 0 );
  return true;
}

bool tx_datagram_fits_user_ntb ( uint16_t datagram_size ) {
  USB_LOG(MOD_TX, "tx_datagram_fits_user_ntb\r\n");
  if ( ncm_if.tx_user_ntb == NULL ) return false;
  if ( ncm_if.tx_user_ntb_datagram_index >= NCM_MAX_DATAGRAMS_PER_NTB ) return false;
  if ( ( ncm_if.tx_user_ntb->nth.wBlockLength + datagram_size + ALIGN_OFFSET(datagram_size) ) > NCM_NTB_MAX_SIZE ) return false;
  USB_LOG(MOD_TX, "\tYes!\r\n");
  return true;
}

bool tx_accept_user_ntb ( void ) {
  USB_LOG(MOD_TX, "tx_accept_user_ntb\r\n");
  if ( ncm_if.tx_user_ntb != NULL ) {
    if (!push_ntb ( &ncm_if.tx_wait_ntb, ncm_if.tx_user_ntb )) {
      USB_LOG(MOD_TX, "\tDoesn't fit into tx_wait_ntb\r\n");
      return false; // No space to push NTB
    }
  }

  ncm_if.tx_user_ntb = pop_ntb ( &ncm_if.tx_free_ntb );

  if ( ncm_if.tx_user_ntb == NULL ) {
    USB_LOG(MOD_TX, "\tNo free slots\r\n");
    return false;
  }

  ncm_if.tx_user_ntb_datagram_index = 0;
  // Populate NTB / Clear Datagram
  ntb_def_t * ntb = (ntb_def_t *)ncm_if.tx_user_ntb;
  ntb->nth.dwSignature = NTH16_SIGNATURE;
  ntb->nth.wHeaderLength = sizeof( ntb->nth );
  ntb->nth.wSequence = ncm_if.tx_seq_count++;
  ntb->nth.wBlockLength = sizeof( ntb->nth ) + sizeof( ntb->ndp ) + sizeof( ntb->ndp_datagram );
  ntb->nth.wNdpIndex = sizeof( ntb->nth );
  ntb->ndp.dwSignature = NDP16_SIGNATURE_NCM0;
  ntb->ndp.wLength = sizeof( ntb->ndp ) + sizeof( ntb->ndp_datagram );
  ntb->ndp.wNextNdpIndex = 0;
  memset( ntb->ndp_datagram, 0, sizeof( ntb->ndp_datagram ) );
  return true;
}

bool tx_try_start ( void ) {
  USB_LOG(MOD_TX, "tx_try_start\r\n");
  if ( ncm_if.tx_usbd_ntb != NULL ) return false;
  if ( ncm_if.interface_active != true ) return false;
  if ( usb_ep_busy ( NCM_EP_DATA_IN ) ) return false;
  ncm_if.tx_usbd_ntb = pop_ntb ( &ncm_if.tx_wait_ntb );
  if ( ncm_if.tx_usbd_ntb == NULL && ( ncm_if.tx_user_ntb == NULL || ncm_if.tx_user_ntb_datagram_index == 0 ) ) {
    USB_LOG(MOD_TX, "\tNothing Waiting\r\n");
    return false;
  }
  if ( ncm_if.tx_usbd_ntb == NULL ) {
    ncm_if.tx_usbd_ntb = ncm_if.tx_user_ntb;
    ncm_if.tx_user_ntb = NULL;
  }
  USB_LOG(MOD_TX, "\tStarting Transfer\r\n");
  return usb_transfer_ep ( NCM_EP_DATA_IN, ncm_if.tx_usbd_ntb->data, ncm_if.tx_usbd_ntb->nth.wBlockLength );
}