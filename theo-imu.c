/*
 * theo-imu.c
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <libusb.h>
#include "libusb-gsource.h"
#include "theo-imu.h"

#define CTRL_IN_EP              0x80
#define CTRL_OUT_EP             0x00
#define INTR_IN_EP              0x81
#define INTR_OUT_EP             0x01
#define BULK_IN_EP              0x82
#define BULK_OUT_EP             0x02
#define ISOC_IN_EP              0x83
#define ISOC_OUT_EP             0x03

#define MAX_PACKET_SIZE         64



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





static void imu_cb(struct libusb_transfer *transfer){
    //unsigned char *buf = transfer->buffer;

    int retErr;
//    int i;
//    int bytes_written;

    switch(transfer->status){
    case LIBUSB_TRANSFER_COMPLETED:
        //write data to socket
#if 0
        switch(sensor){
        case accel:
            write_tagged_message(FOURCC('A', 'C', 'C', '1'));
            break;
        case gyro:
            write_tagged_message(FOURCC('G', 'Y', 'R', 'O'));
            break;
        case magn:
            write_tagged_message(FOURCC('M', 'A', 'G', 'N'));
            break;
        case macc:
            write_tagged_message(FOURCC('A', 'C', 'C', '2'));
            break;
        default:
            write_tagged_message(FOURCC('E', 'R', 'R', 'O'), "Unknown theo-imu sensor id", sizeof("Unknown theo-imu sensor id") - 1);
            break;
        }
#endif
        retErr = libusb_submit_transfer(transfer);
        if(retErr){
            print_libusb_transfer_error(transfer->status, "bulk_in_cb resub");
        }
        break;
    case LIBUSB_TRANSFER_CANCELLED:
        //do nothing.
        break;
    default:
        print_libusb_transfer_error(transfer->status, "bulk_in_cb");
        printf("quit bulk_in\n");
        exit(EXIT_FAILURE);
        break;
    }
}

void init_theo_imu(libusbSource * usb_source){
    int iface_nums[1] = {0};
    struct libusb_transfer * imu_transfer = NULL;
    unsigned char * imu_buf  = calloc(MAX_PACKET_SIZE, sizeof(unsigned char));
    libusb_device_handle * imu_handle = NULL;

    imu_handle = open_usb_device_handle(usb_source, is_imu, iface_nums, 1);

    imu_transfer = libusb_alloc_transfer(0);
    libusb_fill_bulk_transfer(imu_transfer,
                              imu_handle,
                              BULK_IN_EP,
                              imu_buf,
                              MAX_PACKET_SIZE,
                              imu_cb,
                              NULL,
                              0);
}
//libusb_close(imu_handle)
