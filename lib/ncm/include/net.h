#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "desc_types.h"

bool net_can_tx( uint16_t size );
void net_tx( void * datagram, uint16_t arg );
void net_rx_renew( void );
void net_init( void );
bool net_usb_transfer( uint8_t ep_addr, uint16_t bytes );
bool net_usb_control_transfer( usb_control_request_t * request );

extern uint16_t net_tx_cb( void * destination, void * datagram, uint16_t arg );
extern uint16_t net_rx_cb( void * datagram, uint16_t size );