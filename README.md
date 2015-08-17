This is a collection of 1D particle simulations intended as pretty animations
for an LED strip. The program just outputs raw B/R/G (blue, red, green) values
on STDOUT to be piped into /dev/spixxxx for certain kinds of LED strips
connected via SPI to a Raspberry PI. You might have to enable/configure some
GPIO ports in advance.

Between the particles in "waves" there is a repelling force proportional to
the inverse of their squared distance. Sometimes one border particle induces
an impulse to the system and you can see waves propagating and reflecting at
the borders. Sometimes the particle change their color.

Just type "make" to build the program. The only requirement is an installed
C++ compiler. You might want to edit the Makefile for debugging purposes.
