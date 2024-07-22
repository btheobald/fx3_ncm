#include "ncm.h"
#include "usb.h"
#include "net.h"
#include "struct.h"
#include <stdlib.h>

#include "debug.h"
#define MOD_RX "RX"

// TODO: Validate Datagram
bool rx_validate_datagram ( ntb_t * ntb, uint16_t size ) {
  USB_LOG(MOD_RX, "rx_validate_datagram\tAlways Valid\r\n");
  (void)(ntb);
  (void)(size);
  return true;
}

bool rx_update_user_ntb ( void ) {
  USB_LOG(MOD_RX, "rx_update_user_ntb\r\n");
  if ( ncm_if.rx_user_ntb == NULL ) {
    USB_LOG(MOD_RX, "\tFetch user NTB from waiting\r\n");
    ncm_if.rx_user_ntb = pop_ntb( &ncm_if.rx_wait_ntb );
    ncm_if.rx_user_ntb_datagram_index = 0;
  }
  if ( ncm_if.rx_user_ntb != NULL) {
    USB_LOG(MOD_RX, "\tNTB Present\r\n");
    const ndp16_datagram_t *ndp16_datagram = (ndp16_datagram_t *) (ncm_if.rx_user_ntb->data + ncm_if.rx_user_ntb->nth.wNdpIndex + sizeof(ndp16_t));
    uint16_t dIndex = ndp16_datagram[ ncm_if.rx_user_ntb_datagram_index ].wDatagramIndex;
    uint16_t dLength = ndp16_datagram[ ncm_if.rx_user_ntb_datagram_index ].wDatagramLength;
    if ( net_rx_cb ( &(ncm_if.rx_user_ntb->data[dIndex]), dLength ) ) {
      USB_LOG(MOD_RX, "\tOK\r\n");
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
  USB_LOG(MOD_RX, "rx_try_start\r\n");
  if ( ncm_if.rx_usbd_ntb != NULL ) return false;
  if ( ncm_if.interface_active != true ) return false;
  if ( usb_ep_busy ( NCM_EP_DATA_OUT ) ) return false;
  ncm_if.rx_usbd_ntb = pop_ntb ( &ncm_if.rx_free_ntb );
  if ( ncm_if.rx_usbd_ntb == NULL ) {
    USB_LOG(MOD_RX, "\tNo free ntb\r\n");
    return false;
  }
  USB_LOG(MOD_RX, "\tStart Reception\r\n");
  bool r = usb_transfer_ep ( NCM_EP_DATA_OUT, ncm_if.rx_usbd_ntb->data, NCM_NTB_MAX_SIZE );
  if ( !r ) {
    push_ntb ( &ncm_if.rx_free_ntb, ncm_if.rx_usbd_ntb );
    return false;
  }
  return true;
}
