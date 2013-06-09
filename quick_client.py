#!/usr/bin/env python
"""quick_client.py: This will open a socket to
the (probably fake) flight comptuer and read
the data as a smoke test"""

import datetime
import socket
import sys

from fc_proto import packet_header, tag_header, message_types

quiet = (sys.argv[1:] == ['-q'])

# Log all received data
logname = datetime.datetime.now().strftime("log-%Y-%m-%d:%H:%M:%S")
logfile = open(logname, "a")
if not quiet:
    print "logging to", logname

# UDP packets on localhost
UDP_IP   = ""
UDP_PORT = 36000

# Open socket and bind to address
sock = socket.socket(socket.AF_INET,
                     socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

# Listen
while 1:
    message = sock.recv(4096)
    logfile.write(tag_header.pack('SEQN', packet_header.size, 0, 0))
    logfile.write(message)
    if quiet:
        continue

    sequence_number, = packet_header.unpack(message[:packet_header.size])
    message = message[packet_header.size:]

    print "packet sequence %d (%d bytes)" % (sequence_number, len(message))

    while len(message) > 0:
        fourcc, length, timestamp_hi, timestamp_lo = tag_header.unpack(message[:tag_header.size])
        message = message[tag_header.size:]

        body = message[:length]
        message = message[length:]

        timestamp = timestamp_hi << 32 | timestamp_lo
        decoder = message_types.get(fourcc)
        if decoder is not None and len(body) == decoder.size:
            body = decoder.unpack(body)
        print timestamp, repr(fourcc), repr(body)
