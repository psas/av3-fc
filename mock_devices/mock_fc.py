#!/usr/bin/env python
import threading
import time
import e407_roll
import e407_sensor

class RollControl(threading.Thread):

    def __init__(self):
        super(RollControl, self).__init__()
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


roll = RollControl()

try:
    roll.start()
    while 1:
        time.sleep(0.1)
except KeyboardInterrupt, SystemExit:
    roll.stop()
    pass
