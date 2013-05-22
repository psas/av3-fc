/*
 * adis-imu.c
 *
 * Initializes and automatically starts data transfers from theo-imu, and logs
 * the recorded data.
 */

#include <stdlib.h>
#include <stdio.h>
#include <libusb.h>
#include "usbutils.h"
#include "adis-imu.h"
#include "logging.h"
#include "miml.h"


#define BULK0_IN_EP 0x82

#define CTRL_OUT_EP             0x00
#define ADDR_ACC                0x80
#define ADDR_GYR                0x40
#define ADDR_MAG                0x20
#define ADDR_CAC                0x10
#define ADDR_ALL                0xF0
#define INST_RESET              0x01
#define INST_GO                 0x02
#define INST_STOP               0x03
#define INST_INC_SPEED          0x04
#define INST_DEC_SPEED          0x05

#define IMU_INST(X)             ((X) & 0x0F)
#define IMU_ADDR(X)             ((X) & 0xF0)

#define ADIS_PACKET_SIZE 24
#define SENSOR_DATA_OFFSET 6

static int is_adis(libusb_device * device){
    struct libusb_device_descriptor descr;
    int retErr = libusb_get_device_descriptor(device, &descr);
    if(retErr){
        print_libusb_error(retErr,"is_imu libusb_get_device_descriptor");
        return 0;
    }
    if(descr.idVendor == 0xFFFF && descr.idProduct == 0x0009){
        //todo: more ID methods
        return 1;
    }
    return 0;
}

static void common_cb(struct libusb_transfer *transfer, uint32_t fourcc){
    unsigned char *buf = NULL;
    unsigned int act_len;
    int retErr;

    switch(transfer->status){
    case LIBUSB_TRANSFER_COMPLETED:
        buf = transfer->buffer;

        act_len = transfer->actual_length;
        if(act_len != ADIS_PACKET_SIZE){
        	FCF_Log(fourcc, buf, act_len);
        }else{
        	FCF_Log(fourcc, buf, act_len);
        }
        retErr = libusb_submit_transfer(transfer);
        if(retErr){
            print_libusb_transfer_error(transfer->status, "adis_cb resub");
        }
        break;
    case LIBUSB_TRANSFER_CANCELLED:
        //do nothing.
        break;
    default:
        print_libusb_transfer_error(transfer->status, "adis_cb");
        break;
    }
}

static void adis_cb(struct libusb_transfer *transfer){
    if(transfer->actual_length != ADIS_PACKET_SIZE){
        common_cb(transfer, FOURCC('A','D','I','E'));
    }else{
        common_cb(transfer, FOURCC('A','D','I','S'));
    }
}

static void ctrl_cb(struct libusb_transfer *transfer){

}

static int start_bulk_transfer(libusb_device_handle * handle,
        unsigned int ep, libusb_transfer_cb_fn cb, void * data,
        unsigned int timeout)
{
    int iso_packets = 0, usb_err, i, num_urbs_in_flight = 1;
    struct libusb_transfer * trans[num_urbs_in_flight];
    unsigned char * buf = NULL;
    int packet_size = ADIS_PACKET_SIZE;//libusb_get_max_iso_packet_size(device, ep);
    if(packet_size < 0){
        return packet_size;
    }

    for(i = 0; i < num_urbs_in_flight; ++i){
        trans[i] = libusb_alloc_transfer(iso_packets);
        if(trans[i] == NULL){
            for(--i; i >= 0; --i){
                libusb_free_transfer(trans[i]);
            }
            return LIBUSB_ERROR_NO_MEM;
        }
        buf  = calloc(packet_size, sizeof(unsigned char));
        if(buf == NULL){
            for(; i>= 0; --i)
                libusb_free_transfer(trans[i]);
            return LIBUSB_ERROR_NO_MEM;
        }
        libusb_fill_bulk_transfer(trans[i], handle, ep, buf, packet_size, cb,
                data, timeout);

        trans[i]->flags = LIBUSB_TRANSFER_FREE_BUFFER;
        usb_err = libusb_submit_transfer(trans[i]);
        if(usb_err != 0){
            for(--i; i >=0; --i){
                libusb_cancel_transfer(trans[i]);
                //todo: handle error besides LIBUSB_ERROR_NOT_FOUND for cancel
                libusb_free_transfer(trans[i]);
            }
            return usb_err;
        }
    }
    return 0;
}

void init_adis_imu(libusbSource * usb_source){
    int iface_nums[1] = {0};
    libusb_device_handle * imu = open_usb_device_handle(usb_source, is_adis,
            iface_nums, 1);
    if(!imu)
        return;

    struct libusb_transfer * ctrl = libusb_alloc_transfer(0);
    unsigned char * ctrl_buf = calloc(LIBUSB_CONTROL_SETUP_SIZE,
            sizeof(unsigned char));
    libusb_fill_control_setup(ctrl_buf, LIBUSB_RECIPIENT_OTHER |
            LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
            ADDR_ALL | INST_GO, 0, 0, 0);
    libusb_fill_control_transfer(ctrl, imu, ctrl_buf, ctrl_cb, NULL, 0);
    libusb_submit_transfer(ctrl);

    start_bulk_transfer(imu, BULK0_IN_EP, adis_cb, NULL, 0);

}
