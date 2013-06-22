#!/usr/bin/env python
from struct import pack
import random

FILE = "sim_data/adis16405_log.txt-8June2013.csv"


with open(FILE, 'w') as f_out:
    for i in range(100):
        f_out.write('ADIS')
        f_out.write(',')
        f_out.write(chr(0)*6)
        f_out.write(',')
        for i in range(10):
            f_out.write(pack('<H', random.randint(0,2**16-1)))
            f_out.write(',')
        f_out.write('\n')
