#!/usr/bin/env python
from __future__ import print_function
from contextlib import contextmanager
import socket

FC_IP = b'127.0.0.1'
FC_LISTEN_PORT = 36000


@contextmanager
def udp(bind_ip, bind_port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((bind_ip, bind_port))
    sock.settimeout(0.25)
    yield sock
    sock.close()


def talk(msg, from_port):

    with udp(b'0.0.0.0', from_port) as sock:
        sock.sendto(msg.encode('ascii', 'ignore'), (FC_IP, FC_LISTEN_PORT))
        print("Sent", msg, "To FCF")
        
        try:
            data, remote_addr = sock.recvfrom(1024)
            print("Received", repr(data))
        except socket.timeout:
            print ("No FC responce")

