import time
import socket
from fc_proto import packet_header, tag_header, message_types

# UDP packets on localhost
UDP_IP   = ""
UDP_PORT = 5005


#===============================================================================
# Socket Connection
#===============================================================================
class FC_Socket():

    def __init__(self, data):
        self.model = data
        self.socket = self.connect()

    def connect(self):
        # Open socket and bind to address
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((UDP_IP, UDP_PORT))
        return sock

    def listen(self):
        last_seqn = 0
        while 1:
            message = self.socket.recv(4096)
            seqn, = packet_header.unpack(message[:packet_header.size])
            # Read packet
            if seqn:
                self.model.num_packets += 1
                self.model.missed_packets += seqn - (last_seqn+1)
                self.model.miss_percent = self.model.missed_packets / float(self.model.num_packets) * 100
                self.model.packets.append(seqn)

                # Message
                message = message[packet_header.size:]
                if len(message) > 0:
                    fourcc, length, timestamp_hi, timestamp_lo = tag_header.unpack(message[:tag_header.size])
                    message = message[tag_header.size:]

            
            # reinit
            last_seqn = seqn
            

    def fake_recieve(self):
        for i in range(10):
            self.model.num_packets = i
            time.sleep(0.84)
