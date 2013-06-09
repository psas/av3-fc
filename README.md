# PSAS Flight Computer Framework

The Flight Control Framework, or FCF, is a bit of a misnomer in and of itself.
While it was originally developed to run on high-power rockets, the core
framework itself is extendable to work with practically any two pieces of C
code that exchange data. With minimal overhead, this lightweight framework was
designed for fast and efficient data transfer between multiple, interrelated,
but abstracted, pieces of code. This abstraction is created by allowing these
different code fragments, called user modules, to pass data between each other
without referencing one another explicitly. Instead, a relationship between any
two or more user modules is set up in a language called MIML.


## Framework Repository Structure

 - `/` - All the files that comprise the framework, code generator, modules, etc.
 - `/profiler` - External profiler that feeds input through sockets from a remote python script.
 - `/templates` - Basic templates for creating modules that connect to libusb or sockets. Read the in-file instructions for how to properly configure and save to a new module file.


# The Framework

The framework itself is simply a conduit for passing data between code modules.
It's principally made up of the framework file, fcfutils.c, that includes the
main loop and API functions and a collection of sender/receiver relationships,
or intermodular data handlers, in the fcfmain.c file. 


**Read more about the framework here: [github.com/psas/elderberry](https://github.com/psas/elderberry)**


# Building the Flight Computer

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


## Installing


You might need some packages

    $ sudo apt-get install libusb-1.0-0-dev pkg-config

The code generator runs on python3 using the pyyaml package. It's recomended to
use a python virtual environment like this:

    $ sudo apt-get install python3 python-pip virtualenvwrapper
    $ mkvirtualenv -p <path/to/python3/install> av3fc
    (av3fc)$ pip install -r requirements.txt

Then when you want to build the flight computer run

    (av3fc)$ make

### Using the Makefile

As discussed in the introduction to this section, the easiest way to use the
Makefile is to just type `make`.

Here are some other possible uses: `make miml` generates Miml.mk.
`make` builds the project. Then, every repeated use of `make` rebuilds
the project. If one of the ".miml" files changes, make automatically runs
the code generator to rebuild fcfmain.c and fcfmain.h. If the miml files change
so that modules are added or removed, one would have to rebuild the Miml.mk
manually by rerunning `make miml`.


# Profiling

A special profiler module can be added to the system to check latency of the
framework on a particular machine setup. The directions for installing and
using the profiler are as follows:

 1. Hook up the profile module: Edit Main.miml so that the source and messages

sections include the following lines:

```YAML
sources:
- [PROFILE, module_profile.miml]

messages:
PROFILE.sendMessage_profile:
  - PROFILE.getMessage_profile

PROFILE.sendMessage_profile3:
  - PROFILE.getMessage_profile3
```

 1. Run the FC.
 1. After a couple of seconds, the program prints a report message and terminates. The message is in the format: Finished with count: <X> in <Y> sec. <X>, where Y is the time it took to send X dummy messages.
 1. The value of X can be configured by setting MAX_COUNT in module_profile.c. See module_profile.c for details.
