#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "desc_types.h"

extern bool usb_transfer_ep0 ( usb_control_request_t * request, void* buffer, uint16_t bytes );
extern bool usb_transfer_ep ( uint8_t ep_addr, void* buffer, uint16_t bytes );
extern bool usb_control_status( usb_control_request_t * request );
extern bool usb_ep_busy ( uint8_t ep_addr );