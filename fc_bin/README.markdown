# Flight Comptuer bin tools

Since we can send signals to the flight computer, sometimes it's useful to
create those signals while in a terminal on the flight comptuer.

The python program `FC` takes an action as a paramter and sends the
apropriate signal.


## FC Manual

    usage: FC [-h] {disable-servo,enable-servo,safe,slock,override,arm} ...

    optional arguments:
      -h, --help            show this help message and exit

    Flight computer commands:
      {disable-servo,enable-servo,safe,slock,override,arm}
        disable-servo       Disable the servo
        enable-servo        Enable the servo
        safe                Safe the FC
        slock               Enable FC ARM sensor lock
        override            Override the FC ARM sensor lock
        arm                 Put FC in arm state

