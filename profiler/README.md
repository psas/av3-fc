# FakeMouse Profiler
## Dependencies
### C++
+ C++11 compiler (also known as C++0.x)
+ libboost - Asio and Random modules.
### Other
+ Python 3
+ gprof
## Running the program.

First run:

    ./fakemouse \[host\] \[port\]

Where *host* is the target host of the device and *port* is the target port of
the device. This will start to generate random input for the device.

Don't worry about stopping it when the device stops...it will keep checking
the port and retrying.

Then run the profiler:

    ./runprog.py \[times\_list\] \[program and arguments\]

Where *times_list* is a time in the format HH:MM:SS,HH:MM:SS,...,HH:MM:SS and
*program and arguments* is your program + arguments. An example of execution
might look something like this:

    ./runprog.py 00:00:30,00:01:00,04:30:00 ./fc

Here, we're running the program *fc* for 30 seconds; then for 1 minute; and
then for 4 hours, 30 minutes.

## Understanding gprof output.

If everything works out properly, you should see a CSV file in
"profiler-data/output.csv."
