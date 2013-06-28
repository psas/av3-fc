#!/usr/bin/env python
import socket
import config

class RollDevice(object):

    def __init__(self):
        # Open socket and bind to address
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind((config.FC_IP, config.ROLL_RX_PORT))
        self.socket.settimeout(0.1)

    def listen(self):
        try:
            message = self.socket.recv(4096)
            m = config.RC_Servo_Message.unpack(message)
            #print m
        except socket.timeout:
            return

    def close(self):
        self.socket.close()
