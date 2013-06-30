#!/usr/bin/env python
import socket

class fctalk(object):

    FC_IP = '127.0.0.1'
    FC_LISTEN_PORT = 36000

    def __init__(self, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('0.0.0.0', port))

    def send_cmd(self, message):
        self.sock.sendto(message, (self.FC_IP, self.FC_LISTEN_PORT))
        print "Sent", message, "To FCF"
        data, remote_addr = self.sock.recvfrom(4096)
        print "Received", repr(data)

    def close(self):
        self.sock.close()

def the_easy_way(msg, port):
    talk = fctalk(port)
    talk.send_cmd(msg)
    talk.close()
