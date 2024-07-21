#include "tx.h"
#include "ncm.h"
#include "struct.h"
#include <stdlib.h> 

void tx_notification ( bool force_next ) {
  if ( !force_next && ncm_if.notification_state != TX_NOTIFY_STATE_WAITING ) {
    return;
  }
  ncm_if.notification_state = TX_NOTIFY_STATE_SPEED_CHANGE;
  switch ( ncm_if.notification_state ) {
    case TX_NOTIFY_STATE_SPEED_CHANGE:
      ncm_if.notification_state = TX_NOTIFY_STATE_CONNECTED;
      usb_transfer_ep ( NCM_EP_INTERRUPT, &notify_speed, sizeof(notify_speed) );
      break;
    case TX_NOTIFY_STATE_CONNECTED:
      ncm_if.notification_state = TX_NOTIFY_STATE_WAITING;
      usb_transfer_ep ( NCM_EP_INTERRUPT, &notify_connection, sizeof(notify_connection) );
      break;
  }
}

bool tx_insert_zlp ( uint16_t size ) {
  if ( size == 0 || size % NCM_EP_SIZE != 0 ) {
    return false;
  }
  usb_transfer_ep ( NCM_EP_DATA_IN, NULL, 0 );
  return true;
}

bool tx_datagram_fits_user_ntb ( uint16_t datagram_size ) {
  if ( ncm_if.tx_user_ntb == NULL ) return false;
  if ( ncm_if.tx_user_ntb_datagram_index >= NCM_MAX_DATAGRAMS_PER_NTB ) return false;
  if ( ( ncm_if.tx_user_ntb->nth.wBlockLength + datagram_size + ALIGN_OFFSET(datagram_size) ) > NCM_NTB_MAX_SIZE ) return false;
  return true;
}

bool tx_accept_user_ntb ( void ) {
  if ( ncm_if.tx_user_ntb != NULL ) {
    if (!push_ntb ( &ncm_if.tx_wait_ntb, ncm_if.tx_user_ntb )) {
      return false; // No space to push NTB
    }
  }
  ncm_if.tx_user_ntb = pop_ntb ( &ncm_if.tx_free_ntb );
  ncm_if.tx_user_ntb_datagram_index = 0;
  // Populate NTB / Clear Datagram
  ntb_def_t * ntb = (ntb_t *)ncm_if.tx_user_ntb;
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
  if ( ncm_if.tx_usbd_ntb != NULL ) return false;
  if ( ncm_if.interface_active != true ) return false;
  if ( usb_ep_busy ( NCM_EP_DATA_IN ) ) return false;
  ncm_if.tx_usbd_ntb = pop_ntb ( &ncm_if.tx_wait_ntb );
  if ( ncm_if.tx_usbd_ntb == NULL && ( ncm_if.tx_user_ntb == NULL || ncm_if.tx_user_ntb_datagram_index == 0 ) ) {
    return false;
  }
  if ( ncm_if.tx_usbd_ntb == NULL ) {
    ncm_if.tx_usbd_ntb == ncm_if.tx_user_ntb;
    ncm_if.tx_user_ntb == NULL;
  }
  usb_transfer_ep ( NCM_EP_DATA_IN, ncm_if.tx_usbd_ntb->data, ncm_if.tx_usbd_ntb->nth.wBlockLength );
}