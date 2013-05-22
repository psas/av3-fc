/*
 * miml.c
 * auto-generated file
 */

#include "miml.h"
#include "profile.h"

int count = 0;

void FCFERR_Poll() {
        handleErrorPoll();
}

void FCF_Init (libusbSource * src) {
        init_gps(src);
        init_theo_imu(src);
        init_adis_imu(src);
        init_mouse(src);                //read from usb mouse; set your hw values in is_mouse()
        init_mouse2(src);     //read from a 2nd usb mouse; set your hw values in is_mouse2()
        //InitProfiling();
        //init_virtgyro(src);   //read from socket
}

void FCF_Log (uint32_t fourcc, const unsigned char *buf, int act_len) {
  write_tagged_message(fourcc, buf, act_len);
  printbuffer(fourcc, buf, act_len);
  //flush_buffers();
}

void FCF_ProfSendMsg(const char *message){
  profReceiveMsg(message);
}
