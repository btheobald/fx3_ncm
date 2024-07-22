#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ncm.h"

#define ALIGN_OFFSET(x) ((4 - ((x) & (NCM_NDP_ALIGN_MIN_SIZE - 1))) & (NCM_NDP_ALIGN_MIN_SIZE - 1))

typedef struct {
  __attribute__ ((aligned(4))) ntb_t * ntb_list[NCM_NTB_N];
  uint16_t list_front;
  uint16_t list_back;
} ntb_list_t;

typedef enum {
  TX_NOTIFY_STATE_WAITING,
  TX_NOTIFY_STATE_SPEED_CHANGE,
  TX_NOTIFY_STATE_CONNECTED
} notifiy_state_t;

typedef struct {
  uint8_t interface_active;                // Traffic allowed

  // OUT
  ntb_t rx_ntb[NCM_NTB_N];              // Actual NTBs
  ntb_list_t rx_free_ntb;               // NTBs Avaliable
  ntb_list_t rx_wait_ntb;               // NTBs Waiting for User
  ntb_t *rx_usbd_ntb;                   // NTB USB -> Driver
  ntb_t *rx_user_ntb;                   // NTB Driver -> User
  uint16_t rx_user_ntb_datagram_index;  // Current Datagram

  // IN
  ntb_t tx_ntb[NCM_NTB_N];              // Actual NTBs
  ntb_list_t tx_free_ntb;               // NTBs Avaliable
  ntb_list_t tx_wait_ntb;               // NTBs Waiting for USB
  ntb_t *tx_usbd_ntb;                   // NTB Driver -> USB
  ntb_t *tx_user_ntb;                   // NTB User -> Driver
  uint16_t tx_user_ntb_datagram_index;  // Current Datagram
  uint16_t tx_seq_count;

  // Interrupt
  notifiy_state_t notification_state;   // Notification Transmitting
} ncm_interface_t;

extern ncm_interface_t ncm_if;
extern ntb_parameters_t ntb_parameters;
extern usb_cdc_notification_t notify_connection;
extern usb_cdc_notify_speed_t notify_speed;

bool push_ntb ( ntb_list_t * list, ntb_t * ntb );
ntb_t * pop_ntb ( ntb_list_t * list );