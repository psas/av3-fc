#!/usr/bin/env python
"""fake_fc_data.py: This will open a socket and dump
fake data into it for testing ground software being
able to read and understand the flight computer"""

import socket
import struct
import time
import datetime
import random

from fc_proto import packet_header, tag_header

# UDP packets on localhost
UDP_IP   = "127.0.0.1"
UDP_PORT = 5005

# Open socket
sock = socket.socket(socket.AF_INET,
                     socket.SOCK_DGRAM)
sock.connect((UDP_IP, UDP_PORT))

# Make timestamp nanoseconds from midnight
today = datetime.datetime.utcnow()
today = datetime.datetime(today.year, today.month, today.day, 0,0,0)

accel_body = struct.Struct(">HHH")

# Write some random packets in a loop
sequence_num = 0
for i in range(5):
    # Packet header
    packet_type     = "ACC1"
    timestamp       = datetime.datetime.utcnow()
    timestamp       = int((timestamp - today).total_seconds() * 1e9)
    packet_length   = 6

    # Fake data
    ax = random.randint(0,65535)
    ay = random.randint(0,65535)
    az = random.randint(0,65535)

    packet = packet_header.pack(sequence_num) + tag_header.pack(packet_type, accel_body.size, (timestamp >> 32) & 0xFFFF, timestamp & 0xFFFFFFFF) + accel_body.pack(ax, ay, az)

    sock.send(packet)

    # Wait a little
    time.sleep(0.1)
    sequence_num = sequence_num + 1

