#!/usr/bin/env python
import socket
from struct import unpack

# UDP packets on localhost
UDP_IP   = ""
UDP_PORT = 35003

# Open socket and bind to address
sock = socket.socket(socket.AF_INET,
                     socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

# Listen
while 1:
    message = sock.recv(4096)
    m = unpack('!HB', message)
    print m
