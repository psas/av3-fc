# Simulation Of Flight Computer UDP Packets

### Run the fake flight computer:


    $ python fake_fc_data.py --random


This will generate random numbers in the flight computer message
format and pack them into UDP packets and send them on port 5005
on localhost.

### Listen for packets


    $ python quick_client.py


There is also a basic client script that will listen on the port
and dump a log of the incoming packets. It also attempts to read
the binary message data and print human readable numbers to the
screen.
