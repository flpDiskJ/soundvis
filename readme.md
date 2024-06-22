Soundvis V1

Written by Jake Aigner

Simple real-time audio visualizer.



How to use:

First enter the number of the audio device you want to use as an input.

Then the program will open the display window. Screen dimensions can be altered in main.cpp

Keyboard 1-5 changes display type

keyboard B, G, O, R, Y change color

Keyboard Escape quits the program


How to compile:

Ensure Cmake is installed.

	sudo apt install cmake

Ensure SDL2 libraries are installed

	sudo apt install libsdl2-dev

Create and navigate to build directory

	mkdir build
	cd build

Generate cmake makefile

	cmake ..

Compile and run

	make
	./soundvis