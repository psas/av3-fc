#!/usr/bin/python

from fc_proto import tag_header, message_types
import sys

if len(sys.argv) > 1:
    source = open(sys.argv[1])
else:
    source = sys.stdin

while True:
    fourcc, length, timestamp_hi, timestamp_lo = tag_header.unpack(source.read(tag_header.size))
    body = source.read(length)
    timestamp = timestamp_hi << 32 | timestamp_lo
    decoder = message_types.get(fourcc)
    if decoder is not None and len(body) == decoder.size:
        body = decoder.unpack(body)
    print timestamp, repr(fourcc), repr(body)
