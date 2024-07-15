#include <stdint.h>
#include "desc_types.h"

#define CY_FX_EP_NOTIFIER               0x81
#define CY_FX_EP_PRODUCER               0x02    /* EP 2 OUT */
#define CY_FX_EP_CONSUMER               0x82    /* EP 2 IN */

typedef struct {
    usb_config_descriptor_t config_desc;
    // IAD
    usb_interface_assoc_descriptor_t ncm_iad_desc;
    // Interface
    usb_interface_descriptor_t ncm_control_intf;
    usb_cdc_header_desc_t ncm_header_desc;
        usb_cdc_union_desc_t ncm_union_desc;
        usb_cdc_ether_desc_t ecm_desc;
        usb_cdc_ncm_desc_t ncm_desc;
            usb_endpoint_descriptor_t ncm_notify_ep_desc;
    // Datapath
    usb_interface_descriptor_t ncm_data_nop_intf;
    usb_interface_descriptor_t ncm_data_intf;
        usb_endpoint_descriptor_t ncm_in_ep_desc;
        usb_endpoint_descriptor_t ncm_out_ep_desc;
} __attribute__ ((packed)) usb_desc_t;

#define STR_LANG 0x0409
#define STR_MANF u"pureLiFi"
#define STR_PROD u"PnP"
#define STR_INTF u"USB CDC-NCM"
#define STR_MACA u"2EF5F368DCE6"

enum {
  STR_ID_LANG,
  STR_ID_MANF,
  STR_ID_PROD,
  STR_ID_MACA,
  STR_ID_INTF
};

extern const usb_string_descriptor_t str_desc_langid;
extern const usb_string_descriptor_t str_desc_manufacturer;
extern const usb_string_descriptor_t str_desc_product;
extern const usb_string_descriptor_t str_desc_interface;
extern const usb_string_descriptor_t str_desc_serial;

extern const usb_device_descriptor_t usb_device_desc;
extern usb_desc_t usb_config_desc;

extern const usb_cdc_ncm_ntb_parameters_t ntb_parameters;