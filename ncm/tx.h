#pragma once
#include <stdint.h>

void tx_notification ( bool force_next );
bool tx_insert_zlp ( uint16_t size );
bool tx_datagram_fits_user_ntb ( uint16_t datagram_size );
bool tx_accept_user_ntb ( void );
bool tx_try_start ( void );