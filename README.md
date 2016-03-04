# pm3Bruter
EM4x Brute Force mod for Proxmark3

See https://github.com/Proxmark/proxmark3/blob/master/COMPILING.txt for Proxmark3 compiling instructions.
To use image compare features, install fswebcam and imagemagick
sudo apt-get install fswebcam
sudo apt-get install imagemagick

Applying the PMBruter patch
Copy bruter.c, bruter.h, Makefile.patch and proxmark3.c.patch into the proxmark3/client directory.

Run the patch:
patch proxmark3.c < proxmark3.c.patch
patch Makefile < Makefile.patch

make

./proxmark3 -h
syntax: ./proxmark3 <port> [options]

	Linux example:'./proxmark3 /dev/ttyACM0'

	### Proxmark Bruter ###
	-b		Enable brute force function
	-m [mode]	1 = brute force up and down (default), 2 = brute force down, 3 = brute force up
	-c [number]	Number of attempts (default 100)
	-w		Use webcam to identify valid tags (requires fswebcam and imagemagick)
	-p [path]	Path to store images (default '/tmp/')
	-t [UID]	Specify custom tag

	example:'./proxmark3 /dev/ttyACM0 -b -m 3 -c 50 -w -p /root/pictures/ -t 0102030405'
