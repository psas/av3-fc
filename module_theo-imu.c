/*
 * module_theo-imu.c
 *
 * Initializes and automatically starts data transfers from theo-imu, and logs
 * the recorded data.
 */

#include <stdlib.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include "fcfutils.h"
#include "module_theo-imu.h"
#include "utils_libusb-1.0.h"


#define BULK0_IN_EP 0x82
#define BULK1_IN_EP 0x85
#define BULK2_IN_EP 0x88
#define BULK3_IN_EP 0x8B
#define ACC_EP BULK0_IN_EP
#define GYR_EP BULK1_IN_EP
#define MAG_EP BULK2_IN_EP
#define CAC_EP BULK3_IN_EP

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
#define NUM_URBS_IN_FLIGHT 8

static const int VID = 0xFFFF;
static const int PID = 0x0005;

static libusb_device_handle *handle = NULL;
static int ntrans = 0;	//number of elements in trans
static struct libusb_transfer *trans[NUM_URBS_IN_FLIGHT * 4];

static void data_callback(struct libusb_transfer *transfer, const char * src){
    unsigned char *buf = NULL;
    unsigned int act_len;
    int retErr;

    switch(transfer->status){
    case LIBUSB_TRANSFER_COMPLETED:
        buf = transfer->buffer;
        act_len = transfer->actual_length;
        if(act_len != IMU_PACKET_SIZE){
        	sendMessage_theo_imu(src, buf, act_len);
        }else{
            if(IMU_ADDR(buf[0]) == ADDR_GYR){
            	sendMessage_theo_imu(src, buf, act_len);
            }else{
            	sendMessage_theo_imu(src, buf, act_len - 1);
            }
        }
        retErr = libusb_submit_transfer(transfer);
        if(retErr){
            //print_libusb_transfer_error(transfer->status, "imu_cb resub");
        }
        break;

    case LIBUSB_TRANSFER_CANCELLED:
        //do nothing.
        break;
    default:
        //print_libusb_transfer_error(transfer->status, "imu_cb");
        break;
    }
}

static void mag_cb(struct libusb_transfer *transfer){
	data_callback(transfer, "theo_imu_mag");
}
static void acc_cb(struct libusb_transfer *transfer){
    data_callback(transfer, "theo_imu_acc");
}
static void gyr_cb(struct libusb_transfer *transfer){
    data_callback(transfer, "theo_imu_gyr");
}
static void cac_cb(struct libusb_transfer *transfer){
   data_callback(transfer, "theo_imu_cac");
}


static void ctrl_cb(struct libusb_transfer *transfer){

}

static int start_bulk_transfer(libusb_device_handle * handle,
        unsigned int ep, libusb_transfer_cb_fn cb, void * data,
        unsigned int timeout)
{
    int iso_packets = 0, usb_err, i;
    unsigned char * buf = NULL;
    int packet_size = IMU_PACKET_SIZE;
    if(packet_size < 0){
        return packet_size;
    }

    for(i = 0; i < NUM_URBS_IN_FLIGHT; ++i, ++ntrans){
        trans[ntrans] = libusb_alloc_transfer(iso_packets);
        if(trans[ntrans] == NULL){
            for(--i; i >= 0; --i, --ntrans){
                libusb_free_transfer(trans[ntrans-1]);
            }
            return LIBUSB_ERROR_NO_MEM;
        }
        buf  = calloc(packet_size, sizeof(unsigned char));
        if(buf == NULL){
            for(; i>= 0; --i, --ntrans)
                libusb_free_transfer(trans[ntrans]);
            return LIBUSB_ERROR_NO_MEM;
        }
        libusb_fill_bulk_transfer(trans[ntrans], handle, ep, buf, packet_size, cb,
                data, timeout);

        trans[ntrans]->flags = LIBUSB_TRANSFER_FREE_BUFFER;
        usb_err = libusb_submit_transfer(trans[ntrans]);
        if(usb_err != 0){
        	libusb_free_transfer(trans[ntrans]);
            for(--i; i >=0; --i, --ntrans){
                libusb_cancel_transfer(trans[ntrans-1]);
                //todo: handle error besides LIBUSB_ERROR_NOT_FOUND for cancel
                libusb_free_transfer(trans[ntrans-1]);
            }
            return usb_err;
        }
    }
    return 0;
}


void init_theo_imu(){
	handle = open_device("theo_imu", VID, PID);
	
	if (handle == NULL) {
		return;
	}

    struct libusb_transfer * ctrl = libusb_alloc_transfer(0);
    unsigned char * ctrl_buf = calloc(LIBUSB_CONTROL_SETUP_SIZE,
            sizeof(unsigned char));
    libusb_fill_control_setup(ctrl_buf, LIBUSB_RECIPIENT_OTHER |
            LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
            ADDR_ALL | INST_GO, 0, 0, 0);
    libusb_fill_control_transfer(ctrl, handle, ctrl_buf, ctrl_cb, NULL, 0);
    libusb_submit_transfer(ctrl);

	if (handle != NULL) {
		start_bulk_transfer(handle, MAG_EP, mag_cb, NULL, 0);
	    start_bulk_transfer(handle, GYR_EP, gyr_cb, NULL, 0);
	    start_bulk_transfer(handle, ACC_EP, acc_cb, NULL, 0);
	    start_bulk_transfer(handle, CAC_EP, cac_cb, NULL, 0);
	}
}

void finalize_theo_imu() {
	for (int i = 0; i < ntrans; i++) {
		cancel_transfer(trans[i]);
	}
	close_device(handle);
	handle = NULL;
	ntrans = 0;
}
