/*
 * theo-imu.c
 *
 * Initializes and automatically starts data transfers from theo-imu, and logs
 * the recorded data.
 */

#include <stdlib.h>
#include <stdio.h>
#include <libusb.h>
#include "libusb-gsource.h"
#include "theo-imu.h"
#include "logging.h"


#define MAG_EP 0x83
#define GYR_EP 0x87
#define ACC_EP 0x8B
#define CAC_EP 0x8F

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

#define IMU_PACKET_SIZE 13
#define SENSOR_DATA_OFFSET 6

static gboolean is_imu(libusb_device * device){
    struct libusb_device_descriptor descr;
    int retErr = libusb_get_device_descriptor(device, &descr);
    if(retErr){
        print_libusb_error(retErr,"is_imu libusb_get_device_descriptor");
        return FALSE;
    }
    if(descr.idVendor == 0xFFFF && descr.idProduct == 0x0005){
        //todo: more ID methods
        return TRUE;
    }
    return FALSE;
}

static void common_cb(struct libusb_transfer *transfer, uint32_t fourcc){
    unsigned char *buf = NULL;
    const uint16_t len = 6;
    char data[len];
    int retErr, i;

    switch(transfer->status){
    case LIBUSB_TRANSFER_COMPLETED:
        buf = libusb_get_iso_packet_buffer_simple(transfer, 0);
        //USB is little endian but the network is big endian
        data[0] = buf[1 + SENSOR_DATA_OFFSET];//x lsb
        data[1] = buf[0 + SENSOR_DATA_OFFSET];//x msb
        data[2] = buf[3 + SENSOR_DATA_OFFSET];//y lsb
        data[3] = buf[2 + SENSOR_DATA_OFFSET];//y msb
        data[4] = buf[5 + SENSOR_DATA_OFFSET];//z lsb
        data[5] = buf[4 + SENSOR_DATA_OFFSET];//z msb

        write_tagged_message(fourcc, data, len);
        retErr = libusb_submit_transfer(transfer);
        if(retErr){
            print_libusb_transfer_error(transfer->status, "imu_cb resub");
        }
        break;
    case LIBUSB_TRANSFER_CANCELLED:
        //do nothing.
        break;
    default:
        print_libusb_transfer_error(transfer->status, "imu_cb");
        break;
    }
}

static void mag_cb(struct libusb_transfer *transfer){
    common_cb(transfer, FOURCC('M','A','G','N'));
}
static void acc_cb(struct libusb_transfer *transfer){
    common_cb(transfer, FOURCC('A','C','C','1'));
}
static void gyr_cb(struct libusb_transfer *transfer){
    common_cb(transfer, FOURCC('G','Y','R','O'));
}
static void cac_cb(struct libusb_transfer *transfer){
    common_cb(transfer, FOURCC('A','C','C','2'));
}

static void ctrl_cb(struct libusb_transfer *transfer){

}

static int start_iso_transfer(libusb_device_handle * handle,
        unsigned int ep, libusb_transfer_cb_fn cb, void * data,
        unsigned int timeout)
{
    int iso_packets = 1, usb_err, i, num_urbs_in_flight = 8;
    struct libusb_transfer * trans[num_urbs_in_flight];
    unsigned char * buf = NULL;
    int packet_size = IMU_PACKET_SIZE;//libusb_get_max_iso_packet_size(device, ep);
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
        libusb_fill_iso_transfer(trans[i], handle, ep, buf, packet_size,
                iso_packets, cb, data, timeout);
        trans[i]->flags = LIBUSB_TRANSFER_FREE_BUFFER;
        libusb_set_iso_packet_lengths(trans[i], packet_size);
        usb_err = libusb_submit_transfer(trans[i]);
        if(usb_err != 0){
            libusb_free_transfer(trans[i]);
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

void init_theo_imu(libusbSource * usb_source){
    int iface_nums[1] = {0};
    libusb_device_handle * imu = open_usb_device_handle(usb_source, is_imu,
            iface_nums, 1);

    struct libusb_transfer * ctrl = libusb_alloc_transfer(0);
    unsigned char * ctrl_buf = calloc(LIBUSB_CONTROL_SETUP_SIZE,
            sizeof(unsigned char));
    libusb_fill_control_setup(ctrl_buf, LIBUSB_RECIPIENT_OTHER |
            LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
            ADDR_ALL | INST_GO, 0, 0, 0);
    libusb_fill_control_transfer(ctrl, imu, ctrl_buf, ctrl_cb, NULL, 0);
    libusb_submit_transfer(ctrl);

    start_iso_transfer(imu, MAG_EP, mag_cb, NULL, 0);
    start_iso_transfer(imu, GYR_EP, gyr_cb, NULL, 0);
    start_iso_transfer(imu, ACC_EP, acc_cb, NULL, 0);
    start_iso_transfer(imu, CAC_EP, cac_cb, NULL, 0);

}
//todo: how to libusb_close(imu_handle)?
