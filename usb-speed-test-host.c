/*
 * usb-speed-test-host.c
 *
 *  Created on: Jul 24, 2012
 *      Author: theo
 *
 * I apologize in advance, this code is very crude at the moment
 */

#include <stdlib.h>
#include <stdio.h>
#include <libusb.h>
#include <time.h>
#include <poll.h>

#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <signal.h>
#include <linux/usbdevice_fs.h>
#include <sys/ioctl.h>

#include "usb-speed-test-host.h"
#include "libusb-gsource.h"
#include "imu-device-host-interface.h"

//todo: thread safe?
//todo: handle all the errors
//todo: better error handling
//todo: handle device reset/disconnect-reconnect
//todo: printf( to fprintf(stderr,

struct linux_device_handle_priv {
	int fd;
};

struct list_head {
	struct list_head *prev, *next;
};
typedef struct libusbi_device_handle {
	/* lock protects claimed_interfaces */
	pthread_mutex_t lock;
	unsigned long claimed_interfaces;

	struct list_head list;
	struct libusb_device *dev;
	unsigned char os_priv[0];
}libusbi_device_handle;

#define R_ISOC 8
struct libusb_transfer * endpoint[NUM_EPS];
struct libusb_transfer * isoc_out_multi[R_ISOC];

int ttyS0;
libusb_context *imu_host;
//libusbSource * usb_source;
libusb_device_handle *imu_handle;
int iface_nums[NUM_IFACES];
int use_ioctls;
uint8_t request_type;
int initial_ctrl_done;

int is_imu_device(libusb_device * device){
	struct libusb_device_descriptor descr;
	int retErr = libusb_get_device_descriptor(device, &descr);
	if(retErr){
		print_libusb_error(retErr,"is_imu_device libusb_get_device_descriptor");
		return FALSE;
	}

	if(descr.idVendor == 0xFFFF && descr.idProduct == 0x0005){
		//todo: more ID methods
		return TRUE;
	}

	return FALSE;
}

void common_in_cb(struct libusb_transfer *transfer){
	unsigned char *buf = transfer->buffer;

	//int retErr;
	int i;
	int bytes_written;

	switch(transfer->status){
	case LIBUSB_TRANSFER_COMPLETED:
		for(i = 0; i < transfer->actual_length; ++i){
			if(buf[i] == 'A'){
//				printf("U\n");
				bytes_written = write(ttyS0, "U", 1);
				if (bytes_written == -1){
					   perror("Unable to write to ttyS0");
				}
			}
		}
		libusb_submit_transfer(endpoint[BULK_IN_IDX]);
		libusb_submit_transfer(transfer);
		break;
	case LIBUSB_TRANSFER_CANCELLED:
		//do nothing.
		break;
	default:
		print_libusb_transfer_error(transfer->status, "bulk_in_cb");
		printf("quit bulk_in\n");
		raise (SIGINT);
		break;
	}
}
void common_out_cb(struct libusb_transfer *transfer){
	if(transfer->status != LIBUSB_TRANSFER_COMPLETED){
		print_libusb_transfer_error(transfer->status, "bulk_out_cb");
		printf("quit bulk_out\n");
		raise (SIGINT);
	}
}
void ctrl_in_cb(struct libusb_transfer *transfer){
	unsigned char *buf = libusb_control_transfer_get_data(transfer);
	//int retErr;
	int i;
	int bytes_written;

	switch(transfer->status){
	case LIBUSB_TRANSFER_COMPLETED:
		for(i = 0; i < transfer->actual_length; ++i){
			if(buf[i] == 'A'){
				bytes_written = write(ttyS0, "U", 1);
				if (bytes_written == -1){
					   perror("Unable to write to ttyS0");
				}
			}
		}
		libusb_submit_transfer(transfer);
		break;
	case LIBUSB_TRANSFER_CANCELLED:
		//do nothing.
		break;
	default:
		print_libusb_transfer_error(transfer->status, "bulk_in_cb");
		printf("quit bulk_in\n");
		raise (SIGINT);
		break;
	}
}
void ctrl_out_cb(struct libusb_transfer *transfer){
	struct libusb_control_setup * sent = libusb_control_transfer_get_setup(transfer);
	int i;

	//for ioctls
	struct usbdevfs_urb * urb = NULL;
	char * buf = NULL;
	struct libusbi_device_handle * handle = (libusbi_device_handle*)imu_handle;
	struct linux_device_handle_priv * priv = ((struct linux_device_handle_priv *)handle->os_priv);
	int fd = priv->fd;

	if(transfer->status != LIBUSB_TRANSFER_COMPLETED){
		print_libusb_transfer_error(transfer->status, "bulk_out_cb");
		printf("quit bulk_out\n");
		raise (SIGINT);
	}

	switch(sent->bRequest){
	case CTRL_REQ:
		libusb_fill_control_setup(endpoint[CTRL_IN_IDX]->buffer,
			LIBUSB_RECIPIENT_OTHER | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
			READ_DATA, 0, 0, 1);
		libusb_submit_transfer(endpoint[CTRL_IN_IDX]);
		break;
	case INTR_REQ:
		libusb_submit_transfer(endpoint[INTR_IN_IDX]);
		break;
	case BULK_REQ:
		if(use_ioctls){
			initial_ctrl_done = 1;
			urb = calloc(1, sizeof(struct usbdevfs_urb));
			buf = calloc(MAX_PACKET_SIZE, sizeof(char));
			//urb->usercontext;
			urb->type = USBDEVFS_URB_TYPE_BULK;
			urb->endpoint = BULK_IN_EP;
			//urb->flags;
			urb->buffer_length = MAX_PACKET_SIZE;
			urb->buffer = buf;
			ioctl(fd, USBDEVFS_SUBMITURB, urb);
		}else{
			libusb_submit_transfer(endpoint[BULK_IN_IDX]);
		}
		break;
	case ISOC_REQ:
		for(i = 0; i < R_ISOC; ++i)
			libusb_submit_transfer(isoc_out_multi[i]);
		break;
	default:
		break;
	}
}
void isoc_in_cb(struct libusb_transfer *transfer){
	unsigned char *buf = libusb_get_iso_packet_buffer_simple(transfer, 0);
	//int retErr;
	int i;
	int bytes_written;

	switch(transfer->status){
	case LIBUSB_TRANSFER_COMPLETED:
		for(i = 0; i < transfer->iso_packet_desc[0].actual_length; ++i){
			if(buf[i] == 'A'){
//				printf("U\n");
				bytes_written = write(ttyS0, "U", 1);
				if (bytes_written == -1){
					   perror("Unable to write to ttyS0");
				}
			}
		}
		libusb_submit_transfer(transfer);
		//libusb_submit_transfer(endpoint[BULK_OUT_EP]);
		break;
	case LIBUSB_TRANSFER_CANCELLED:
		//do nothing.
		break;
	default:
		print_libusb_transfer_error(transfer->status, "bulk_in_cb");
		printf("quit bulk_in\n");
		raise (SIGINT);
		break;
	}
}
void libusb_mainloop_error_cb(int timeout, int handle_events){
	if(timeout)
		print_libusb_error(timeout, "libusb timeout");
	if(handle_events)
		print_libusb_error(handle_events, "libusb handle_events");
	printf("quit libusb\n");
	raise (SIGINT);
}

void open_ttyS0(){
	//int ttyS0;
	struct termios attrib;

	ttyS0 = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
	if (ttyS0 == -1){
		perror("open_port: Unable to open /dev/ttyS0");
		raise(SIGINT);
	}
	tcgetattr(ttyS0, &attrib);
	cfsetispeed(&attrib, B115200);
	tcsetattr(ttyS0, TCSANOW, &attrib); //todo: valgrind errors here
}

struct libusb_transfer * allocate_default_transfer(libusb_device_handle * handle,
		struct libusb_endpoint_descriptor * ep, libusb_transfer_cb_fn callback)
{
	//this function is not used currently
	unsigned char * buf = malloc(ep->wMaxPacketSize);;
	struct libusb_transfer * transfer = libusb_alloc_transfer(0);

	transfer->dev_handle = handle;
	transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;
	transfer->endpoint = ep->bEndpointAddress;
	transfer->type = ep->bmAttributes & 0x03;

	switch(ep->bmAttributes & 0x03){
	case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
	case LIBUSB_TRANSFER_TYPE_INTERRUPT:
	case LIBUSB_TRANSFER_TYPE_BULK:

		break;
	default:
		break;
	}
	transfer->timeout = 0; //todo: interrupt transfers?
	transfer->length = ep->wMaxPacketSize;
	transfer->callback = callback;
	transfer->user_data = NULL;
	transfer->buffer = buf;

	return transfer;
}

void setup_transfers(libusb_device_handle *imu_handle){
	int i;
	unsigned char * ctrl_in_buffer = NULL;
	unsigned char * ctrl_out_buffer = NULL;
	unsigned char * intr_in_buffer = NULL;
	unsigned char * bulk_in_buffer = NULL;

	unsigned char * isoc_mulit_xfer[R_ISOC] = {NULL};

	//allocate and default fill transfers
		endpoint[CTRL_IN_IDX]  = libusb_alloc_transfer(0);
		endpoint[CTRL_OUT_IDX] = libusb_alloc_transfer(0);
		endpoint[INTR_IN_IDX]  = libusb_alloc_transfer(0);
		endpoint[BULK_IN_IDX]  = libusb_alloc_transfer(0);

		ctrl_in_buffer  = calloc(MAX_PACKET_SIZE + LIBUSB_CONTROL_SETUP_SIZE, sizeof(unsigned char));
		ctrl_out_buffer = calloc(MAX_PACKET_SIZE + LIBUSB_CONTROL_SETUP_SIZE, sizeof(unsigned char));
		intr_in_buffer  = calloc(MAX_PACKET_SIZE, sizeof(unsigned char));
		bulk_in_buffer  = calloc(MAX_PACKET_SIZE, sizeof(unsigned char));

		//todo:set free buffer flag in transfers.

		libusb_fill_control_transfer(endpoint[CTRL_IN_IDX],
									 imu_handle,
									 ctrl_in_buffer,
									 ctrl_in_cb,
									 NULL,
									 0);
		libusb_fill_control_transfer(endpoint[CTRL_OUT_IDX],
									 imu_handle,
									 ctrl_out_buffer,
									 ctrl_out_cb,
									 NULL,
									 0);

		libusb_fill_interrupt_transfer(endpoint[INTR_IN_IDX],
									   imu_handle,
									   INTR_IN_EP,
									   intr_in_buffer,
									   MAX_PACKET_SIZE,
									   common_in_cb,
									   NULL,
									   0);

		libusb_fill_bulk_transfer(endpoint[BULK_IN_IDX],
								  imu_handle,
								  BULK_IN_EP,
								  bulk_in_buffer,
								  MAX_PACKET_SIZE,
								  common_in_cb,
								  NULL,
								  0);

		for(i = 0; i < R_ISOC; ++i){
			isoc_out_multi[i] = libusb_alloc_transfer(1);
			isoc_mulit_xfer[i]  = calloc(MAX_PACKET_SIZE, sizeof(unsigned char));
			libusb_fill_iso_transfer(isoc_out_multi[i],
									 imu_handle,
									 ISOC_IN_EP,
									 isoc_mulit_xfer[i],
									 MAX_PACKET_SIZE,
									 1,
									 isoc_in_cb,
									 NULL,
									 0);

			libusb_set_iso_packet_lengths(isoc_out_multi[i], 1);
		}

}

void print_help(){
	printf("\
Options:\n\
 -h  print this\n\
 -c  use control transfers\n\
 -i  use interrupt transfers\n\
 -b  use bulk transfers\n\
 -bi use bulk ioctls\n\
 -s  use isocrhonus transfers\n\n\
 C-c to stop");
}

void handle_cmd_arguments(char* arg){
	if (!strcmp(arg, "-h")){
		print_help();
		raise(SIGINT);
	}else
	if (!strcmp(arg, "-c")){
		request_type = CTRL_REQ;
	}else
	if (!strcmp(arg, "-i")){
		request_type = INTR_REQ;
	}else
	if (!strcmp(arg, "-b")){
		request_type = BULK_REQ;
	}else
	if (!strcmp(arg, "-bi")){
		use_ioctls = 1;
		request_type = BULK_REQ;
	}else
	if (!strcmp(arg, "-s")){
		request_type = ISOC_REQ;
	}else{
		fprintf(stderr, "UNKNOWN COMMAND\n");
		raise(SIGINT);
	}
}
void termination_handler (int signum){
	//cleanup
	int usbErr, i;

	libusb_control_transfer(imu_handle,
			LIBUSB_RECIPIENT_OTHER | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
			STOPPED, 0, 0, NULL, 0, 500);

	for(i = 0; i < NUM_EPS; ++i){
		if(endpoint[i]){
			libusb_cancel_transfer(endpoint[i]);
			libusb_free_transfer(endpoint[i]);
		}
	}

	for(i = 0; i < R_ISOC; ++i){
		if(isoc_out_multi[i]){
			libusb_cancel_transfer(isoc_out_multi[i]);
			libusb_free_transfer(isoc_out_multi[i]);
		}
	}

	//todo: release all interfaces
	usbErr = libusb_release_interface(imu_handle, iface_nums[0]);
	if(usbErr)
		print_libusb_error(usbErr, "exit libusb_release_interface");
	usbErr = libusb_attach_kernel_driver(imu_handle, iface_nums[0]);
	if(usbErr)
		print_libusb_error(usbErr, "exit libusb_attach_kernel_driver");
	libusb_close(imu_handle);
	//libusbSource_free(usb_source);
	libusb_exit(imu_host);
	if(close(ttyS0))
		perror("closing ttyS0");
	signal (signum, SIG_DFL);
    raise (signum);
}

void mainloop(){

	int r, bytes_written;
	struct usbdevfs_urb * urb;
	char * buf;
	struct libusbi_device_handle * handle = (libusbi_device_handle*)imu_handle;
	struct linux_device_handle_priv * priv = ((struct linux_device_handle_priv *)handle->os_priv);
	int fd = priv->fd;
	struct pollfd urb_fd;
	urb_fd.events = POLLOUT;
	urb_fd.fd = fd;
	urb_fd.revents = 0;

	while(1){
		if(use_ioctls && initial_ctrl_done){
			poll(&urb_fd, 1, -1);

			ioctl(fd, USBDEVFS_REAPURBNDELAY, &urb);

			buf = urb->buffer;
			if(urb->actual_length > 0){
				if(buf[0] == 'A'){
					bytes_written = write(ttyS0, "U", 1);
					if (bytes_written == -1){
						   perror("Unable to write to ttyS0");
					}
				}
			}
			//free(urb);
			//free(buf);
			buf[0] = 0;

			//urb = calloc(1, sizeof(struct usbdevfs_urb));
			//buf = calloc(MAX_PACKET_SIZE, sizeof(char));
			//urb->usercontext;
			urb->type = USBDEVFS_URB_TYPE_BULK;
			urb->endpoint = BULK_IN_EP;
			//urb->flags;
			urb->buffer_length = MAX_PACKET_SIZE;
			urb->buffer = buf;
			ioctl(fd, USBDEVFS_SUBMITURB, urb);

		}else{
			r = libusb_handle_events(imu_host);
			if(r){
				print_libusb_error(r, "mainloop");
				raise(SIGINT);
			}
		}
	}

}



int main(int argc, char *argv[]){
	char in;
	int usbErr = 0;
	iface_nums[0]= 0; // test interface
	if(argc == 2)
		handle_cmd_arguments(argv[1]);
	else{
		print_help();
		raise(SIGINT);
	}
	open_ttyS0();

	//initialize libusb
	usbErr = libusb_init(&imu_host);
	if(usbErr){
		print_libusb_error(usbErr, "libusb_init");
		exit(EXIT_FAILURE);
	}
	libusb_set_debug(imu_host, 3);
	//usb_source = libusbSource_new(imu_host);

	//get the usb device
	do{
		imu_handle = open_usb_device_handle(imu_host, is_imu_device, iface_nums, NUM_IFACES);
		if(!imu_handle){
			printf("**imu_handle acquisition error, retry (y/n)?\n");
			in = getchar();
			while(getchar() != '\n');
			if(in != 'y'){
				printf("quitting\n");
				//libusbSource_free(usb_source);
				libusb_exit(imu_host);
				exit(EXIT_FAILURE);
			}
		}
	}while(!imu_handle);

//    if (signal(SIGINT, termination_handler) == SIG_IGN)
//      signal(SIGINT, SIG_IGN);

//   if(!use_ioctls)
    	setup_transfers(imu_handle);

	printf("beginning main loop\n");


	struct libusb_transfer * ctrl_out = endpoint[CTRL_OUT_IDX];
	libusb_fill_control_setup(ctrl_out->buffer,
			LIBUSB_RECIPIENT_OTHER | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
			request_type, 0, 0, 0);
	libusb_submit_transfer(ctrl_out);
	mainloop();

	raise(SIGINT);
	return 0;
}


