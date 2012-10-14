#!/usr/bin/env python
"""quick_client.py: This will open a socket to
the (probably fake) flight comptuer and read
the data as a smoke test"""

import socket
import struct

from fc_proto import packet_header, tag_header

# UDP packets on localhost
UDP_IP   = ""
UDP_PORT = 5005

message_types = {
    'GPS\x01': struct.Struct("<BBHdddfffffHH H"),
    'GPS\x02': struct.Struct("<LLHHHH H"),
    'GPS\x4C': struct.Struct("<dH8x 81L 15L 12L H"),
    'GPS\x5F': struct.Struct("<HxxL 10L 10L 10L H"),
    'GPS\x60': struct.Struct("<xxHd " + ''.join(("BBBB" for i in range(12))) + "12L 12d 12d H"),
    'GPS\x61': struct.Struct("<L 4H 3L 2H H"),
    'GPS\x62': struct.Struct("<" + ''.join(("HBBBBbB" for i in range(8))) + "BBxx H"),
    'GPS\x63': struct.Struct("<BBHd" + ''.join(("8B b 3B H 5h" for i in range(12))) + "hH H"),
}

# Open socket and bind to address
sock = socket.socket(socket.AF_INET,
                     socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

# Listen
while 1:
    message = sock.recv(4096)
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
