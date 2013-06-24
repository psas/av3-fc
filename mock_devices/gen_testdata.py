#!/usr/bin/env python
from struct import pack
from random import randint

filename = "sim_data/test.ADIS.bin"

with open(filename,'w') as f_out:
    for i in range(10000):
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
        f_out.write(pack('<12H', *values))
        f_out.write('\n')
