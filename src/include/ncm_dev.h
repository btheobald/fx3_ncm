#pragma once

#include <stdint.h>

// Network Driver has finished with packet
void network_notify(void);

// Return if network driver can accept another packet
bool network_ready(uint16_t size);

// Send packet on network
void network_send(void *ref, uint16_t arg);

// Handle Receive from network driver
bool network_recv(const uint8_t *src, uint16_t size);

// Handle Send to network driver
uint16_t network_send(uint8_t *dst, void *ref, uint16_t ar);

// Init network state
void network_init(void);

