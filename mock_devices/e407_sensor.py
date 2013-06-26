import socket
from random import randint, gauss
import datetime
import time
import config

RAD2DEG = 57.2957795
MSS2GEE = 1.0/9.81

class SensorDevice(object):

    def __init__(self, OR_file=None):
        # Open socket and bind to address
        self.ADISsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.ADISsocket.bind((config.SENSOR_IP, config.ADIS_TX_PORT))

        self.apogee = False
        self.launch_time = datetime.datetime.now() + datetime.timedelta(seconds=3)

        print self.launch_time
        self.simdata = None
        # Open Rocket file, if exists:
        if OR_file:
            try:
                self.simdata = open(OR_file, 'r')
            except:
                print "Couldn't open sim file"
                self.simdata = None


    def from_file(self):
        now = datetime.datetime.now()
        if now < self.launch_time:
            return (3.3,0,0,0,9.8,0,0,250,0,0,32.2,0)

        line = self.simdata.readline()
        if "# Event APOGEE occurred" in line:
            self.apogee = True
        if line[0] != '#':
            li = line.split(',')
            if len(li) > 18:
                t       = float(li[ 0])
                x_rate  = float(li[16])  #r/s
                y_rate  = float(li[17])  #r/s
                z_rate  = float(li[18])  #r/s
                x_acc   = float(li[ 3])  #m/s/s
                y_acc   = float(li[11])  #m/s/s
                z_acc   = float(li[11])  #m/s/s
                grav    = float(li[14])  #m/s/s

                ## body frame rotation
                x_rate = -x_rate
                if self.apogee:
                    x_acc   = -(x_acc - grav)
                else:
                    x_acc   = -(x_acc + grav)
                return (3.3, x_rate, y_rate, z_rate, x_acc, y_acc, z_acc, 250,0,0,32.2,0)
        else:
            return self.from_file()


    def gen_random(self):
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
        return values


    def ADIS(self, values):
        newval = range(12)
        
        # Power supply
        newval[0] = int(values[0]/0.002418)

        # Gryo
        newval[1] = int((values[1]*RAD2DEG)/0.05)
        newval[2] = int((values[2]*RAD2DEG)/0.05)
        newval[3] = int((values[3]*RAD2DEG)/0.05)

        # accel
        newval[4] = int((values[4]*MSS2GEE)/0.00333)
        newval[5] = int((values[5]*MSS2GEE)/0.00333)
        newval[6] = int((values[6]*MSS2GEE)/0.00333)

        # mag
        newval[7] = int(values[7]/0.5)
        newval[8] = int(values[8]/0.5)
        newval[9] = int(values[9]/0.5)

        # temp
        newval[10] = int(values[10]/0.14)
        
        # spare ADC
        newval[11] = int(values[11])


        #twos' compliment
        for i, n in enumerate(newval):
            negative = 0
            if n < 0:
                negative = 1

            n = abs(n)

            if negative:
                newval[i] = (n^0xff) + 1
            else:
                newval[i] = n
        return newval

    def noise(self, values):
        newval = range(12)

         # Power supply
        newval[0] = values[0] + gauss(0,0.1)
        # Gryo
        newval[1] = values[1] + gauss(0,0.001)
        newval[2] = values[2] + gauss(0,0.001)
        newval[3] = values[3] + gauss(0,0.001)

        # accel
        newval[4] = values[4] + gauss(0,0.5)
        newval[5] = values[5] + gauss(0,0.5)
        newval[6] = values[6] + gauss(0,0.5)

        # mag
        newval[7] = values[7] + gauss(0,1)
        newval[8] = values[8] + gauss(0,1)
        newval[9] = values[9] + gauss(0,1)

        # temp
        newval[10] = values[10] + gauss(0,0.2)
        
        # spare ADC
        newval[11] = 0

        return newval

    def talk(self):

        if self.simdata:
            values = self.from_file()
            values = self.noise(values)
            values = self.ADIS(values)
        else:
            values = self.gen_random()


        #print values
 
        packet  = config.ADIS_Header.pack('ADIS', 0, 0, config.ADIS_Message.size)
        packet += config.ADIS_Message.pack(*values)

        self.ADISsocket.sendto(packet, (config.FC_IP, config.FC_LISTEN_PORT))
        time.sleep(0.01)

    def close(self):
        self.ADISsocket.close()
        if self.simdata:
            self.simdata.close()
