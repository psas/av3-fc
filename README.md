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


## 1.1 Framework Repository Structure

 - `/` - All the files that comprise the framework, code generator, modules, etc.
 - `/profiler` - External profiler that feeds input through sockets from a remote python script.
 - `/templates` - Basic templates for creating modules that connect to libusb or sockets. Read the in-file instructions for how to properly configure and save to a new module file.


# 2 The Framework

The framework itself is simply a conduit for passing data between code modules.
It's principally made up of the framework file, fcfutils.c, that includes the
main loop and API functions and a collection of sender/receiver relationships,
or intermodular data handlers, in the fcfmain.c file. 


## 2.1 API

User hardware modules connect to the framework by passing in their file descriptors to the `fcf_add_fd(...)` API function in their initialize function. These file descriptors are essential for telling the framework that they have data to pass to other user modules. If the need arises that a module needs to disconnect from the framework, the `fcf_remove_fd(...)` API function fulfills this purpose. 

In rare cases where a user module must end program execution, a third API function, `fcf_stop_main_loop(...)` is used. This function will stop the framework from iterating over another polling loop and will consequently begin the process of methodically shutting down the application. It is important to note that any user module has the ability to call `fcf_stop_main_loop(...)`.

## 2.2 Polling

Once the user modules have registered themselves with the framework using the `fcf_add_fd(...)` API function, the "main loop," or "polling loop," of the framework checks to see which file descriptors are active. When an active file descriptor is found, the polling callback function located in the respective user module is called.

## 2.3 MIML and Sender/Receiver Relationships

To allow modules to pass data between each other without having explicit reference to each other, the framework contains two other components to facilitate this: the MIML language and a code generator. 

For user modules, MIML is used to detail the data-passing, initialization and finalize functions, as well as its header and object file. The data-passing functions can either be considered "senders" or "receivers," depending on the direction of data flow. Senders, which are not part of the user module itself but are auto-generated using the code generator, initiates the calls to the the receivers.

In a separate file called Main.miml, these user module MIML files are listed as "sources" and coupled with a unique identifier. The intermodular data passing is defined using simple binary hierarchy, headed by the sender and subordinated by the receiver functions.

Once the relationships are set up, the code generator parses the MIML files to create C files, fcfmain.c and fcfmain.h that contain the intermodular data handlers, as well as a Makefile include, Miml.mk.


# 3: USER MODULES

While user modules have been referenced many times in this document already, they haven't been formally introduced. A user module is a piece of code created by the user that is utilized in conjunction with the framework that serves a specific and usually unique purpose of the application. Examples of user modules may include code that reads from a GPS device off of USB, code that takes in data and prints to disk, or code that holds the state of a game.

There are two general categories of user modules, "software" and "hardware." 

Hardware user modules are those that connect to physical devices outside the framework and provide the necessary code to interface and pass data to and from them. Hardware user modules connect to the framework by passing file descriptors into the system to be polled.

Software user modules generally do not use file descriptors, but are instead called into via their receiver functions by other software or hardware user modules. However, software modules that need execution time independent of other modules can use timerfds, or file descriptors that are read at specified time intervals, to fit in the polling paradigm of hardware user modules.

## 3.1 User Module Conventions

There are a few conventions that will make user modules work more seamlessly with the framework. 

* Tokens: Every module should have a unique identifier that is somewhat descriptive of what it is, but keeps its functions from having namespace collisions. Possible examples include "mouse1" or "diskLogger."

* Initialize functions: Every user module should have an initialization function. Whether it's setting up dynamically allocated data, referencing a secondary API or library, or simply doing nothing, MIML requires that every user modules have an initialize function. Since its called from auto-generated code, the initialize function should not take any arguments. A recommended naming scheme is:  `void init_<module token>(void);`

* Finalize functions: Every modules should have a finalize function. This function is used to execute code necessary to shut down the user module before the application terminates, such as deallocating any dynamic memory. Since its called from auto-generated code, the finalize function should not take any arguments. A recommended naming scheme is:  `void finalize_<module token>(void);`

* Data-passing functions: Receiver functions should at least contain the token of the user module it's being defined in. A recommended naming scheme is: `void get<function name>_<module token>(args);` An example might be `void getMessage_diskLogger(char *buf);` Sender functions, which are references to the respective auto-generated intermodular data handler are best defined as: `void send<function name>_<module token>(args);` An example might be `void sendMessage_diskLogger(char *buf);` User modules may have multiple sender and receiver functions.

* Filenames: A recommended naming scheme for the user modules is module_<module token>.c and .h. An example might be "module_diskLogger.c" and "module_diskLogger.h".

## 3.2 Helper Files

To help speed up user module creation and reduce duplicated code clutter, helper files can be included to keep common code out of user modules that use the same bus interface. Included in the framework are two helper files, utils_libusb-1.0.c and utils_sockets.c, and their respective headers that can greatly aid in making user modules that interface with libusb or use socket code for data transfers. Both of these helper files are also used in templates.

## 3.3 Templates

For purposes of quickly creating user modules based on libusb or sockets, there are two templates in the "templates" directory that can be modified to create unique instances of user modules. To use the templates, its best to open up the desired file in a text editor and read its directions in the code file itself. They are created so that the user can search-and-replace the ###DEVTAG### value with a unique token in both the .c and .h file and save it with a unique filename (see Section 3.1 for filename conventions).


# 4: BUILDING THE APPLICATION

Due to the need for user module abstraction, the build process for the framework is a little more complicated than that of a typical C application. Here is the general build process:

1. User module MIML files and the Main.miml file are passed into the code generator.
2. The code generator, upon successful parsing and validation, creates the fcfmain.c file that include the intermodular data handlers and a Miml.mk Makefile include file.
3. The Makefile imports the Miml.mk and should successfully compile, link and build the executable "fc."

To help uncomplicate this process, however, the Makefile has been created so that the user only needs to run:

    $ make

For the project to complete all three steps.


### 4.0.1 Installing


You might need some packages

    $ sudo apt-get install libusb-1.0-0-dev pkg-config

The code generator runs on python3 using the pyyaml package. It's recomended to
use a python virtual environment like this:

    $ sudo apt-get install python3 python-pip virtualenvwrapper
    $ mkvirtualenv -p <path/to/python3/install> av3fc
    (av3fc)$ pip install -r requirements.txt

Then when you want to build the flight computer run

    (av3fc)$ make


## 4.1 Creating MIML Files

As a helper to the user, a script called header2Miml.py is used to parse the header file of a user module and create the corresponding MIML file. To use this script, however, each function prototype in the header file should have in an inline comment beside it a special miml directive detailing its role in the form "[miml:<role>]" where role can be init, final, receiver or sender . Here is an example of a header file with the special miml directive in the comments:

```C
extern void init_diskLogger(void); // [miml:init]
extern void finalize_diskLogger(void); // [miml:final]
extern void getMessage_diskLogger(char *buf); // [miml:receiver]
extern void sendMessage_diskLogger(char *buf); // [miml:sender]
```

The user can run either ./header2Miml <user module header file> <user module miml file> to take in the header and output the miml file. An example would be:

    ./header2Miml.py module_diskLogger.h module_diskLogger.miml

As a short cut ./header2Miml <user module filename without extension>. An example would be:

    ./header2Miml.py module_diskLogger

Otherwise, MIML files can be created manually (see separate documentation regarding MIML in documentation folder).

## 4.2 Using the Makefile

As discussed in the introduction to this section, the easiest way to use the Makefile is to just type "make". 

Here are some other possible uses: "make miml" generates Miml.mk. "make" builds the project. Then, every repeated use of "make" rebuilds the project. If one of the ".miml" files changes, make automatically runs the code generator to rebuild fcfmain.c and fcfmain.h. If the miml files change so that modules are added or removed, one would have to rebuild the Miml.mk manually by rerunning "make miml".


# 5: PROFILING

A special profiler module can be added to the system to check latency of the framework on a particular machine setup. The directions for installing and using the profiler are as follows:

* Hook up the profile module: Edit Main.miml so that the source and messages sections include the following lines:

sources:
- [PROFILE, module_profile.miml]

messages:
PROFILE.sendMessage_profile:
  - PROFILE.getMessage_profile

PROFILE.sendMessage_profile3:
  - PROFILE.getMessage_profile3

* Run the FC.
* After a couple of seconds, the program prints a report message and terminates. The message is in the format: Finished with count: <X> in <Y> sec. <X>, where Y is the time it took to send X dummy messages.
* The value of X can be configured by setting MAX_COUNT in module_profile.c. See module_profile.c for details.
