####################
PSAS Flight Computer
####################

.. sidebar:: Contents

    .. toctree::
        :maxdepth: 1

        structure
        hardware


PSAS_ is a student aerospace engineering project at Portland State University.
We are building ultra-low-cost, open source rockets that feature some of the
most sophisticated amateur rocket avionics systems out there today.

The primary flight computer is the brains of the rocket, and our playground for
testing future rocket technology. It's written in C with an event loop model.

We made a custom code generator to do all the event glue and boilerplate,
allowing us to focus on writing functionality.


Install
=======

Requirements
-----------

The code generator runs on python3 using the pyyaml_ package. It's recommended
to use a python virtual environment like this: ::

    $ sudo apt-get install python3 libyaml-0-2 python-pip virtualenvwrapper
    $ mkvirtualenv -p <path/to/python3/install> av3fc
    (av3fc)$ pip install -r requirements.txt

.. note:: If this is your first time using python virtual environments,
          remember to kill your shell and open a new one after installing
          virtualenvwrapper for the first time (you only have to do this
          once).


Building
--------

Due to the need for user module abstraction, the build process for the framework
is a little more complicated than that of a typical C application. Here is the
general build process:

#. User module MIML files and the Main.miml file are passed into the code
   generator.
#. The code generator, upon successful parsing and validation, creates the
   fcfmain.c file that include the intermodular data handlers and a Miml.mk 
   Makefile include file.
#. The Makefile imports the Miml.mk and should successfully compile, link and
   build the executable "fc."

To help uncomplicate this process, however, the Makefile has been created so
that the user only needs to run::

    (av3fc)$ make

For the project to complete all three steps.


Running
=======

The finished executable will be in the top level directory and is called 'fc'.
To start the flight computer run::

    $ ./fc

It will start and wait for some kind of sensor input or command.

.. _PSAS: http://psas.pdx.edu/
.. _pyyaml: http://pyyaml.org/
