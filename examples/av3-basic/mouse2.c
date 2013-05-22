/*
 * mouse2.c (based on theo-imu.c)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "usbutils.h"
#include "mouse2.h"
#include "miml.h"

static const int g_dev_IN_EP = 0x81;
static int g_packet_size = 4;

//Is device second USB mouse yes/no.
static int is_mouse2(libusb_device * device){
	struct libusb_device_descriptor descr;
	int retErr = libusb_get_device_descriptor(device, &descr);
	if(retErr){
		print_libusb_error(retErr,"is_imu libusb_get_device_descriptor");
		return 0;
	}
	if(descr.idVendor == 0x045E && descr.idProduct == 0x00E1) {
		//ID 045e:00e1 Microsoft Corp. Wireless Laser Mouse 6000 Reciever
		return 1;
	}
	//add your IDs here
//	if(descr.idVendor == 0xFFFF && descr.idProduct == 0xFFFF){
//		return 1;
//	}

	return 0;
}


static void common_cb(struct libusb_transfer *transfer, uint32_t fourcc){
    unsigned char *buf = NULL;
    int act_len;
    int retErr;

    switch(transfer->status){
    case LIBUSB_TRANSFER_COMPLETED:

//        printf_tagged_message(FOURCC('L', 'O', 'G', 'S'), "\nJM3 transfer completed");
//        flush_buffers();

        buf = transfer->buffer;
        act_len = transfer->actual_length;

        FCF_Log(fourcc, buf, act_len);	// send message

        //write_tagged_message(fourcc, buf, act_len);

//        if(act_len != g_mouse_packet_size){
//            write_tagged_message(fourcc, buf, act_len);
//        }else{
////            if(IMU_ADDR(buf[0]) == ADDR_GYR){
////                write_tagged_message(fourcc, buf, act_len);
////            }else{
//                write_tagged_message(fourcc, buf, act_len -1);
////            }
//        }
        retErr = libusb_submit_transfer(transfer);
        if(retErr){
            print_libusb_transfer_error(transfer->status, "common_cb resub");
        }
        break;
    case LIBUSB_TRANSFER_CANCELLED:
        //do nothing.
        break;
    default:
        print_libusb_transfer_error(transfer->status, "common_cb");
        break;
    }
}

//libusb_transfer has completed
static void mouse_cb(struct libusb_transfer *transfer){
    if(transfer->actual_length != g_packet_size){
        common_cb(transfer, FOURCC('M','S','2','E'));
    }else{
        common_cb(transfer, FOURCC('M','S','2','N'));
    }
}

static int start_mouse_transfer(libusb_device_handle * handle,
        unsigned int ep, libusb_transfer_cb_fn cb, void * data,
        unsigned int timeout)
{
    int iso_packets = 0, usb_err, i, num_urbs_in_flight = 8;

    num_urbs_in_flight = 1;	//JM

    struct libusb_transfer * trans[num_urbs_in_flight];
    unsigned char * buf = NULL;


    for(i = 0; i < num_urbs_in_flight; ++i){
        trans[i] = libusb_alloc_transfer(iso_packets);
        if(trans[i] == NULL){
            for(--i; i >= 0; --i){
                libusb_free_transfer(trans[i]);
            }
            return LIBUSB_ERROR_NO_MEM;
        }
        buf  = calloc(g_packet_size, sizeof(unsigned char));
        if(buf == NULL){
            for(; i>= 0; --i)
                libusb_free_transfer(trans[i]);
            return LIBUSB_ERROR_NO_MEM;
        }

        libusb_fill_interrupt_transfer(trans[i], handle, ep, buf, g_packet_size, cb, data, timeout);

        trans[i]->flags = LIBUSB_TRANSFER_FREE_BUFFER;
        usb_err = libusb_submit_transfer(trans[i]);
        if(usb_err != 0){

            printf_tagged_message(FOURCC('L', 'O', 'G', 'S'), "\n JM  error ");
            flush_buffers();

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


void init_mouse2(libusbSource * usb_source){
	int iface_nums[1] = {0};
	libusb_device_handle * handle = open_usb_device_handle(usb_source, is_mouse2, iface_nums, 1);
	if(!handle) {
		return;
	}

	g_packet_size = libusb_get_max_packet_size (libusb_get_device (handle), g_dev_IN_EP);

	start_mouse_transfer(handle, g_dev_IN_EP, mouse_cb, NULL, 0);
}
