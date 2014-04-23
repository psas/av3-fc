# PSAS Flight Computer

[![Build Status](https://travis-ci.org/psas/av3-fc.png)](https://travis-ci.org/psas/av3-fc)

# Documentation

[PSAS Flight computer documentation](http://psas-flight-computer.readthedocs.org/).

# Building the Flight Computer

## Requirments 

The code generator runs on python3 using the pyyaml package. It's recomended to
use a python virtual environment like this:

    $ sudo apt-get install python3 libyaml-0-2 python-pip virtualenvwrapper
    # Close the tab and re-open it if virtualenvwrapper was installed.
    $ mkvirtualenv -p `which python3` av3fc
    $ 
    (av3fc)$ pip install -r requirements.txt

Use `deactivate` to get out of the virtual environmnet. To get back into the virtual environment:

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
    fcfmain.c file that include the intermodular data handlers and a Miml.mk 
    Makefile include file.
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
