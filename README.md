# PSAS Flight Computer

[![Build Status](https://travis-ci.org/psas/av3-fc.png)](https://travis-ci.org/psas/av3-fc)


# Building the Flight Computer

## Requirments 

The code generator runs on python3 using the pyyaml package. It's recomended to
use a python virtual environment like this:

    $ sudo apt-get install python3 libyaml-0-2 python-pip virtualenvwrapper
    $ mkvirtualenv -p <path/to/python3/install> av3fc
    (av3fc)$ pip install -r requirements.txt


## Building

Due to the need for user module abstraction, the build process for the framework
is a little more complicated than that of a typical C application. Here is the
general build process:

 1. User module MIML files and the Main.miml file are passed into the code 
    generator.
 1. The code generator, upon successful parsing and validation, creates the
    fcfmain.c file that include the intermodular data handlers and a Miml.mk 
    Makefile include file.
 1. The Makefile imports the Miml.mk and should successfully compile, link and
    build the executable "fc."

To help uncomplicate this process, however, the Makefile has been created so
that the user only needs to run:

    $ make

For the project to complete all three steps.

### Using the Makefile

As discussed in the introduction to this section, the easiest way to use the
Makefile is to just type `make`.

Here are some other possible uses: `make miml` generates Miml.mk.
`make` builds the project. Then, every repeated use of `make` rebuilds
the project. If one of the ".miml" files changes, make automatically runs
the code generator to rebuild fcfmain.c and fcfmain.h. If the miml files change
so that modules are added or removed, one would have to rebuild the Miml.mk
manually by rerunning `make miml`.


## Avionics Network Information

### Flight Computer:

 - IP Addr: `10.0.0.10`
 - MAC Addr: ?
 - Listen Port: `36000`


### Sensor Board:

 - IP Addr: `10.0.0.20`
 - MAC Addr: `E6:10:20:30:40:11`
 - Listen Port: `35001`
 - Transmit Ports:
   - ADIS: `35020`
   - MPL: `35010`
   - MPU: `35002`


### Roll Board:

 - IP Addr: `10.0.0.30`
 - MAC Addr: `E6:10:20:30:40:aa`
 - Listen Port: `35003`
 - Transmit Port: `35004`

