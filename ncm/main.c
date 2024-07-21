#include <ncm.h>
#include <usb.h>
#include <net.h>
#include <debug.h>

#define MOD_MAIN "MAIN"
#define MOD_USB "USB"
#define MOD_CB "CB"

int main() {
    net_init();
    net_can_tx(1000);
    net_tx(NULL, 0);
    net_can_tx(1000);
    net_tx(NULL, 0);
    net_rx_renew();
}

uint16_t net_tx_cb( void * destination, void * datagram, uint16_t arg ) {
    USB_LOG(MOD_CB, "net_tx_cb\r\n");
}

uint16_t net_rx_cb( void * datagram, uint16_t arg ) {
    USB_LOG(MOD_CB, "net_rx_cb\r\n");
}

bool usb_transfer_ep0 ( usb_control_request_t * request, void* buffer, uint16_t bytes ) {
    USB_LOG(MOD_USB, "usb_transfer_ep0\r\n");
}

bool usb_transfer_ep ( uint8_t ep_addr, void* buffer, uint16_t bytes ) {
    USB_LOG(MOD_USB, "usb_transfer_ep\r\n");
}

bool usb_control_status( usb_control_request_t * request ) {
    USB_LOG(MOD_USB, "usb_control_status\r\n");
}

bool usb_ep_busy ( uint8_t ep_addr ) {
    USB_LOG(MOD_USB, "usb_ep_busy\r\n");
}