#!/usr/bin/env python
"""quick_client.py: This will open a socket to
the (probably fake) flight comptuer and read
the data as a smoke test"""

import socket
import struct

# UDP packets on localhost
UDP_IP   = ""
UDP_PORT = 5005

# Open socket and bind to address
sock = socket.socket(socket.AF_INET,
                     socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

# Listen
while 1:
    message = sock.recv(4096)
    print struct.unpack('L4sQHHHH', message)
