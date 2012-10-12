/*
 * libusb-gsource.c
 *
 *  Created on: Jul 14, 2012
 *      Author: theo
 */

#include <libusb.h>
#include <time.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libusb-util.h"

//todo: document
//todo: are there error types I can handle here?
//todo: multithread libusb_handle_events_timeout_completed


libusb_device * find_usb_device(libusb_context * context, is_device is_device){
	libusb_device **list = NULL;
	libusb_device *found = NULL;
	ssize_t num_usb_dev = 0;
	ssize_t i = 0;

	num_usb_dev = libusb_get_device_list(context, &list);
	if(num_usb_dev < 0){
		print_libusb_error(num_usb_dev, "Could not get device list");
		return NULL;
	}
	//look through the list for the device matching is_device
	for(i = 0; i < num_usb_dev; ++i){
		if(is_device(list[i]) == true){
			found = list[i];
			libusb_ref_device(found);
			break;
		}
	}
	if(!found){
		fprintf(stderr, "Device not found\n");
	}

	libusb_free_device_list(list, 1);
	return found;
}

//todo: I dont think I'll be opening multiple interfaces very often. simplify?
libusb_device_handle * open_device_interface(libusb_device * dev, int * iface_num, int num_ifaces){
	libusb_device_handle *handle = NULL;
	int i = 0;
	int retErr = 0;
	int kd_stat = 0;
	if(!dev)
		return NULL;
	retErr = libusb_open(dev, &handle);

	if(retErr){
		print_libusb_error(retErr, "Could not open device");
		return NULL;
	}
	//claim requested interfaces on the device
	for(i=0; i < num_ifaces; ++i){
		//if the kernel driver is active on the interfaces we want, detach it
		kd_stat = libusb_kernel_driver_active(handle, iface_num[i]);
		if(kd_stat < 0){
			print_libusb_error(kd_stat,"Failure finding kernel driver status");
			libusb_close(handle);
			return NULL;
		}
		if(kd_stat > 0){ //the kernel driver is active (kd_stat = 1)
			retErr = libusb_detach_kernel_driver(handle, iface_num[i]);
			if(retErr){
				print_libusb_error(retErr, "Could not detach kernel driver");
				libusb_close(handle);
				return NULL;
			}
		}

		retErr = libusb_claim_interface(handle, iface_num[i]);
		if(retErr){
			print_libusb_error(retErr, "Could not claim device interface");
			libusb_attach_kernel_driver(handle, iface_num[i]);
			libusb_close(handle);
			return NULL;
		}
	}
	return handle;
}

libusb_device_handle * open_usb_device_handle(libusb_context * context,
    is_device is_device, int * iface_num, int num_ifaces)
{
	libusb_device * dev =  find_usb_device(context, is_device);
	libusb_device_handle * handle = open_device_interface(dev, iface_num,num_ifaces);
	libusb_unref_device(dev); //remove the ref from find_usb_device
	return handle;
}

void print_libusb_error(int libusberrno, const char* str) {
	switch(libusberrno) {
    case LIBUSB_SUCCESS:
		fprintf(stderr, "**%s: SUCCESS\n",str);
		break;
    case LIBUSB_ERROR_IO:
		fprintf(stderr, "**%s: ERROR_IO\n",str);
		break;
	case LIBUSB_ERROR_INVALID_PARAM:
		fprintf(stderr, "**%s: ERROR_INVALID_PARAM\n",str);
		break;
	case LIBUSB_ERROR_ACCESS:
		fprintf(stderr, "**%s: ERROR_ACCESS\n",str);
		break;
	case LIBUSB_ERROR_NO_DEVICE:
		fprintf(stderr, "**%s: ERROR_NO_DEVICE\n",str);
		break;
	case LIBUSB_ERROR_NOT_FOUND:
		fprintf(stderr, "**%s: ERROR_NOT_FOUND\n",str);
		break;
	case LIBUSB_ERROR_BUSY:
	   fprintf(stderr, "**%s: ERROR_BUSY\n",str);
	   break;
    case LIBUSB_ERROR_TIMEOUT:
		fprintf(stderr, "**%s: ERROR_TIMEOUT\n",str);
		break;
	case LIBUSB_ERROR_OVERFLOW:
		fprintf(stderr, "**%s: ERROR_OVERFLOW\n",str);
		break;
	case LIBUSB_ERROR_PIPE:
		fprintf(stderr, "**%s: ERROR_PIPE\n",str);
		break;
	case LIBUSB_ERROR_INTERRUPTED:
		fprintf(stderr, "**%s: ERROR_INTERRUPTED\n",str);
		break;
	case LIBUSB_ERROR_NO_MEM:
		fprintf(stderr, "**%s: ERROR_NO_MEM\n",str);
		break;
	case LIBUSB_ERROR_NOT_SUPPORTED:
		fprintf(stderr, "**%s: ERROR_NOT_SUPPORTED\n",str);
		break;
	case LIBUSB_ERROR_OTHER:
		fprintf(stderr, "**%s: ERROR_OTHER\n",str);
		break;
	default:
		fprintf(stderr, "***%s:  unknown error %i ***\n", str, libusberrno);
		break;
    }
/*  fprintf(stderr, "**%s: %s, %d\n", str, libusb_error_name(libusberrno),
 *	        libusberrno);
 *  libusb_error_name() only occurs in libusb1.0.9, 1.0.8 is common
 */
}

void print_libusb_transfer_error(int status, const char* str){
	switch(status){
	case LIBUSB_TRANSFER_COMPLETED:
		fprintf(stderr, "**%s: LIBUSB_TRANSFER_COMPLETED\n", str);
		break;
	case LIBUSB_TRANSFER_ERROR:
		fprintf(stderr, "**%s: LIBUSB_TRANSFER_ERROR\n", str);
		break;
	case LIBUSB_TRANSFER_TIMED_OUT:
		fprintf(stderr, "**%s: LIBUSB_TRANSFER_TIMED_OUT\n", str);
		break;
	case LIBUSB_TRANSFER_CANCELLED:
		fprintf(stderr, "**%s: LIBUSB_TRANSFER_CANCELLED\n", str);
		break;
	case LIBUSB_TRANSFER_STALL:
		fprintf(stderr, "**%s: LIBUSB_TRANSFER_STALL\n", str);
		break;
	case LIBUSB_TRANSFER_NO_DEVICE:
		fprintf(stderr, "**%s: LIBUSB_TRANSFER_NO_DEVICE\n", str);
		break;
	case LIBUSB_TRANSFER_OVERFLOW:
		fprintf(stderr, "**%s: LIBUSB_TRANSFER_OVERFLOW\n", str);
		break;
	default:
		fprintf(stderr, "***%s: Unknown transfer status %i***\n", str, status);
		break;
	}
}
