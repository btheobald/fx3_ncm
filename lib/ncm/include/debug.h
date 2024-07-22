#pragma once
#include <stdio.h>
#include "cyu3system.h"

//#ifdef NCM_LOG
#define USB_LOG(m, ...) CyU3PDebugPrint(4, (char *)m ": " __VA_ARGS__) //printf(m ": " __VA_ARGS__)
//#else
//#define USB_LOG(m, ...)
//#endif