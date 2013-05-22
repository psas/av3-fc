av3-basic
==========

This code is based on the PSAS av3-code found here:
http://git.psas.pdx.edu/av3-fc.git

The goal of this test program is to take the av3-fc code and
* remove the glib dependency,
* implement a basic poll(2) loop, and
* read some data from a USB mouse.

Use lsusb(8) with the -v option to get the vendor- and
product ID of your mouse. Add them to is_mouse() in mouse.c


