#!/usr/bin/env python
import threading
import time
import sys
import e407_roll
import e407_sensor

class RollControl(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._stop = threading.Event()
        self.mock_roll = e407_roll.RollDevice()
        self.daemon = True

    def run(self):
        print "Starting Roll Control Device..."
        while (not self._stop.is_set()):
            self.mock_roll.listen()

    def stop(self):
        print "Stopping Roll Control Device..."
        self._stop.set()
        self.mock_roll.close()
        self.join()


class Sensor(threading.Thread):

    def __init__(self, OR_file):
        threading.Thread.__init__(self)
        self._stop = threading.Event()
        self.mock_sensor = e407_sensor.SensorDevice(OR_file)
        self.daemon = True

    def run(self):
        print "Starting Sensor Device..."
        while (not self._stop.is_set()):
            self.mock_sensor.talk()

    def stop(self):
        print "Stopping Sensor Device..."
        self._stop.set()
        self.mock_sensor.close()
        self.join()

OR_file = None

if len(sys.argv) > 1:
    OR_file = sys.argv[1]

sens = Sensor(OR_file)
roll = RollControl()

try:
    roll.start()
    sens.start()
    while 1:
        time.sleep(0.5)
except KeyboardInterrupt, SystemExit:
    roll.stop()
    sens.stop()
    pass
