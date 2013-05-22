===================================================================================
		     FCF DEMO: ROCK, PAPER, SCISSORS
===================================================================================

1. Find two, 3-button mice and connect them to your system.

2. Change the VID and PID values in both mouse.c and mouse2.c to match the 
VID and PID of your two mice. (For help finding vid/pid, use lsusb -v)

3. Ensure mouse devices have write permissions. 
(sudo chown ugo+w /dev/bus/usb/001/00*)

4. The game records the first valid button push from both mice and compares them.
The ROCK/PAPER/SCISSORS are mapped to the primary three buttons:
	
	- Left Mouse Button:	ROCK
	- Middle Mouse Button:	PAPER
	- Right Mouse Button:	SCISSORS

5. The game will continue until one player's score reaches the NUMTOWIN constant.
(default 5)
