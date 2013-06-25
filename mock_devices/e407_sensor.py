import socket
from random import randint
import time
import config

class SensorDevice(object):

    def __init__(self):
        # Open socket and bind to address
        self.ADISsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.ADISsocket.bind((config.SENSOR_IP, config.ADIS_TX_PORT))

    def talk(self):
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
 
        packet  = config.ADIS_Header.pack('ADIS', 0, 0, config.ADIS_Message.size)
        packet += config.ADIS_Message.pack(*values)

        self.ADISsocket.sendto(packet, (config.FC_IP, config.FC_LISTEN_PORT))
        time.sleep(0.01)

    def close(self):
        self.ADISsocket.close()
