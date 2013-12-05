#################
Project Structure
#################

::

    .
    |-- build_for_flight.sh (use on flight computer)
    |-- cg.conf (code gen configuation)
    |-- doc (project documentation)
    |   |-- ...
    |-- elderberry (Event loop code gen submodule)
    |   |-- ...
    |-- fc_bin (Executable commands like ARM and SAFE)
    |   |-- ...
    |-- main.miml (project configuation)
    |-- Makefile
    |-- README.md
    |-- requirements.txt
    `-- src (FC source code)
        |-- devices (device data handling)
        |   |-- ...
        | -- utilities (helpers and includes)
        |   |-- ...
        | -- main code
        `-- ...

