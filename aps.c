#include "aps.h"

#include <libusb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static libusb_device_handle * aps_handle = NULL;

static gboolean is_aps(libusb_device * device){
    struct libusb_device_descriptor descr;
    int retErr = libusb_get_device_descriptor(device, &descr);
    if(retErr){
        print_libusb_error(retErr,"is_aps libusb_get_device_descriptor");
        return false;
    }
    if(descr.idVendor == 0xFFFF && descr.idProduct == 0x0006){
        //todo: more ID methods
        return TRUE;
    }
    return FALSE;
}

void set_port(int port, uint32_t val){
    int setport = 0x80;
    unsigned char data[64];
    int usb_err;
    data[0] = (val & 0xFF<<0)>>0;
    data[1] = (val & 0xFF<<8)>>8;
    data[2] = (val & 0xFF<<16)>>16;
    data[3] = (val & 0xFF<<24)>>24;
    usb_err = libusb_control_transfer(aps_handle,
            LIBUSB_RECIPIENT_OTHER | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
            setport | port, 0, 0, data, 4, 2000);
    if(usb_err < 0){
        print_libusb_error(usb_err, "set_port");
    }
    if(usb_err != 4){
        printf("set_port: Didn't send correct number of bytes");
    }
}

void clear_port(int port, uint32_t val){
    int clearport = 0x40;
    unsigned char data[64];
    int usb_err;
    data[0] = (val & 0xFF<<0)>>0;
    data[1] = (val & 0xFF<<8)>>8;
    data[2] = (val & 0xFF<<16)>>16;
    data[3] = (val & 0xFF<<24)>>24;
    usb_err = libusb_control_transfer(aps_handle,
            LIBUSB_RECIPIENT_OTHER | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
            clearport | port, 0, 0, data, 4, 2000);
    if(usb_err < 0){
        print_libusb_error(usb_err, "set_port");
    }
    if(usb_err != 4){
        printf("set_port: Didn't send correct number of bytes");
    }
}

void init_aps(libusbSource * usb_source)
{
    int iface_nums[1] = {0};
    aps_handle = open_usb_device_handle(usb_source, is_aps, iface_nums, 1);
    if(aps_handle == NULL){
        exit(1);
    }

    set_port(0, (1<<ATV_SPS_PIN) | (1<<RC_POWER_PIN) | (1<<WIFI_POWER_PIN));
}
