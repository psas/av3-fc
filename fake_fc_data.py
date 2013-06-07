#!/usr/bin/env python
"""fake_fc_data.py: This will open a socket and dump
fake data into it for testing ground software being
able to read and understand the flight computer"""

import socket
import struct
import time
import datetime
from random import randint, choice, uniform, gauss
import sys,os
from fc_proto import packet_header, tag_header, message_types

# CONFIG
# UDP packets on localhost
UDP_IP   = "127.0.0.1"
UDP_PORT = 5005


# Program Modes:
gen_random = True   #default mode
run_file = False


def print_help():
    "List commands"
    print " usage:"
    print ""
    print "    --random         generate random data"
    print "    --file filename  use an OpenRocket csv"
    print "                     file as a data source"
    print ""


# Read flags
if len(sys.argv) > 1:
    if 'random' in sys.argv[1]:
        gen_random = True
    elif 'file' in sys.argv[1]:
        if len(sys.argv) > 2:
            print sys.argv[2]
        else:
            print "Please provide a file"
            print_help()
            exit()
    elif 'help' in sys.argv[1]:
        print_help()
        exit()
    else:
        print_help()
        exit()


# Open socket
sock = socket.socket(socket.AF_INET,
                     socket.SOCK_DGRAM)


# Make timestamp nanoseconds random time in the past
today = datetime.datetime.utcnow()
today = today - datetime.timedelta(0,randint(600,10800))

# Write some random packets in a loop
sequence_num = randint(1234,5678)


class message(object):
    """Message base class, inherit this and override
    self.pack_body() with body packing and make sure
    self.packet_type is defined"""

    @classmethod
    def make_instance(cls, packet_type):
        instance = cls()
        instance.packet_type = packet_type
        return instance

    def generate(self):
        timestamp = datetime.datetime.utcnow()
        timestamp = int((timestamp - today).total_seconds() * 1e9)

        packet = self.pack_body()
        return tag_header.pack(self.packet_type, len(packet),
            (timestamp >> 32) & 0xFFFF, timestamp & 0xFFFFFFFF) + packet

    def pack_body(self):
        return os.urandom(message_types[self.packet_type].size)

class ADIS(message):
    packet_type = 'ADIS'
    def pack_body(self):

        values = (
            randint(0,65535),   # Power supply measurement
            randint(0,65535),   # X-axis gyroscope output
            randint(0,65535),   # Y-axis gyroscope output
            randint(0,65535),   # Z-axis gyroscope output
            randint(0,65535),   # X-axis accelerometer output
            randint(0,65535),   # Y-axis accelerometer output
            randint(0,65535),   # Z-axis accelerometer output
            randint(0,65535),   # X-axis magnetometer measurement
            randint(0,65535),   # Y-axis magnetometer measurement
            randint(0,65535),   # Z-axis magnetometer measurement
            randint(0,65535),   # Temperature output
            randint(0,65535),   # Auxiliary ADC measurement
        )

        body = message_types[self.packet_type].pack(*values)
        return body

class GPS_BIN1(message):
    packet_type = 'GPS\x01'

    def pack_body(self):
        values = (
            randint(     0,   255),     # Age of differential [seconds]
            randint(     0,    12),     # Number of satellites
            randint(     0, 65535),     # GPS week
            uniform(   0.0, 604800.0),  # GPS Time Of Week [seconds]
            uniform(  43.65,    43.75), # Latitude [decimal deg]
            uniform(-120.5,   -120.4),  # Longitude [decimal deg]
            uniform(1000.0,   5000.0),  # Height [meters]
            gauss(     0.0,     10.0),  # Velocity North [m/s]
            gauss(     0.0,     10.0),  # Velocity East [m/s]
            uniform(   0.0,    350.0),  # Velocity Up [m/s]
            uniform(   5.0,     11.0),  # Standard deviation of residuals [meters]
            choice([0,1,2,4,8,16,32]),  # Navigation mode: see integration manual
            randint(     0,    65535),  # Extended Age of differential [seconds]
        )

        body = message_types[self.packet_type].pack(*values)
        return body


#print repr(ACCL().generate())

messages = [
    ADIS(),
    message.make_instance('MPU9'),
    message.make_instance('MPL3'),
    GPS_BIN1()
]

while(1):

    packet = packet_header.pack(sequence_num)
    while len(packet) < 1000:
        packet += choice(messages).generate()

    sock.sendto(packet, (UDP_IP, UDP_PORT))

    # Wait a little
    time.sleep(0.01)
    sequence_num = sequence_num + 1

