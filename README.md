# PSAS Flight Computer

[![Build Status](https://travis-ci.org/psas/av3-fc.png)](https://travis-ci.org/psas/av3-fc)

[Portland State Aerospace Society](http://psas.pdx.edu/) is an open source
"space program" in Portland Oregon.  We build and fly state-of-the-art
rocket avionics systems. We've been testing ways to fly code on small-ish
rockets for over 15 years.

This repo contains our current primary flight computer (an Intel Atom processor
at the heart of a full stack of sensors, batteries, radios, and other electronics. This
all fits in our 5.5-inch diameter, 12-foot tall, high power rocket that flies
over straight up over 4 kilometers.

We've been refining the code to fly a roll control experiment in July 2014.

 - [More information about upcoming launch](https://github.com/psas/Launch-11)
 - [Watch a video from past launches](https://www.youtube.com/user/psasrockets)
 - [Follow us on twitter @pdxaerospace](https://twitter.com/pdxaerospace)


# Documentation

The flight code is written in C, and features a generated `main` function and a
callback-based event loop. The [event loop code generator was written by a senior
CS student group in 2013](https://github.com/psas/elderberry).

This abstraction lets us write small pieces of behavior with very little
boilerplate. The goal is to be able to hand out well-defined projects such as
"write the parser for the GPS messages" to members without having to expect them
to maintain the complicated interconnections to the rest of the system.

 - [PSAS Flight computer documentation (WIP)](http://psas-flight-computer.readthedocs.org/).

This idea is very similar system to NASA's
'[Core Flight Executive](http://code.nasa.gov/project/core-flight-executive-cfe/)'
system for spacecraft software abstraction.


# Building the Flight Computer

## Requirements 

The code generator runs on python3 using the pyyaml package. It's recommended to
use a python virtual environment like this:

    $ sudo apt-get install python3 libyaml-0-2 python-pip virtualenvwrapper
    # Close the tab and re-open it if virtualenvwrapper was installed.
    $ mkvirtualenv -p `which python3` av3fc
    $ 
    (av3fc)$ pip install -r requirements.txt

Use `deactivate` to get out of the virtual environment. To get back into the virtual environment:

    $ workon av3fc

Don't forget to initialize your submodules:

    $ git submodule update --init

## Building

Due to the need for user module abstraction, the build process for the framework
is a little more complicated than that of a typical C application. Here is the
general build process:

 1. User module MIML files and the Main.miml file are passed into the code 
    generator.
 1. The code generator, upon successful parsing and validation, creates the
    fcfmain.c file that includes the intermodular data handlers and a Miml.mk 
    Makefile includes file.
 1. The Makefile imports the Miml.mk and should successfully compile, link and
    build the executable "fc."

To help uncomplicate this process, however, the Makefile has been created so
that the user only needs to run:

    $ make

BUT don't forget you need to be in your virtual environment for this to work (`workon av3fc`).

### Using the Makefile

As discussed in the introduction to this section, the easiest way to use the
Makefile is to just type `make`.

Here are some other possible uses: `make miml` generates Miml.mk.
`make` builds the project. Then, every repeated use of `make` rebuilds
the project. If one of the ".miml" files changes, make automatically runs
the code generator to rebuild fcfmain.c and fcfmain.h. If the miml files change
so that modules are added or removed, one would have to rebuild the Miml.mk
manually by rerunning `make miml`.
