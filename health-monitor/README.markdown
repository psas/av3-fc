# Flight Computer Health Monitor

One requirement for the flight computer is having some kind of health data in
the telemetry stream. In particular we'd like to know about how hard the CPU
is working and something about disk space and performance as well as network
performance.

Most of the reads needed to get this information are blocking, so we have
a separate process (i.e. this one) to collect these values and forward them
in a network packet to the main flight computer process.

See [issue #2](https://github.com/psas/av3-fc/issues/2)


## Install

    pip install -r requirements.txt


## Run

    ./monitor.py
