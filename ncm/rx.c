#include "ncm.h"
#include "struct.h"
#include <stdlib.h>

// TODO: Validate Datagram
bool rx_validate_datagram ( ntb_t * ntb, uint16_t size ) {
  return true;
}

bool rx_update_user_ntb ( void ) {
  if ( ncm_if.rx_user_ntb == NULL ) {
    ncm_if.rx_user_ntb = pop_ntb( &ncm_if.rx_wait_ntb );
    ncm_if.rx_user_ntb_datagram_index = 0;
  }
  if ( ncm_if.rx_user_ntb != NULL) {
    const ndp16_datagram_t *ndp16_datagram = (ndp16_datagram_t *) (ncm_if.rx_user_ntb->data + ncm_if.rx_user_ntb->nth.wNdpIndex + sizeof(ndp16_t));
    uint16_t dIndex = ndp16_datagram[ ncm_if.rx_user_ntb_datagram_index ].wDatagramIndex;
    uint16_t dLength = ndp16_datagram[ ncm_if.rx_user_ntb_datagram_index ].wDatagramLength;
    if ( net_rx_cb ( ncm_if.rx_user_ntb->data + dIndex, dLength ) ) {
      dIndex = ndp16_datagram[ ncm_if.rx_user_ntb_datagram_index + 1 ].wDatagramIndex;
      dLength = ndp16_datagram[ ncm_if.rx_user_ntb_datagram_index + 1 ].wDatagramLength;
      if ( dIndex != 0 && dLength != 0 ) {
        ncm_if.rx_user_ntb_datagram_index++;
      } else {
        return push_ntb( &ncm_if.rx_free_ntb, ncm_if.rx_user_ntb );
      }
    }
  }
  return true;
}

bool rx_try_start ( void ) {
  if ( ncm_if.rx_usbd_ntb != NULL ) return false;
  if ( ncm_if.interface_active != true ) return false;
  if ( usb_ep_busy ( NCM_EP_DATA_OUT ) ) return false;
  ncm_if.rx_usbd_ntb = pop_ntb ( &ncm_if.rx_free_ntb );
  if ( ncm_if.rx_usbd_ntb == NULL ) {
    return false;
  }
  bool r = usb_transfer_ep ( NCM_EP_DATA_OUT, ncm_if.rx_usbd_ntb->data, NCM_NTB_MAX_SIZE );
  if ( !r ) {
    push_ntb ( &ncm_if.rx_free_ntb, ncm_if.rx_usbd_ntb );
    return false;
  }
  return true;
}
