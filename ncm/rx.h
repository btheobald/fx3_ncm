#include "ncm.h"
#include "struct.h"
#include <stdlib.h>

// TODO: Validate Datagram
bool rx_validate_datagram ( ntb_t * ntb, uint16_t size );
bool rx_update_user_ntb ( void );
bool rx_try_start ( void );