# What's this?

Eventually this repository will contain C and Python code I'm running on my Pi Zero W to stream music from Spotify to a bluetooth speaker.
Since I bought a 2.8" capacitive touch display from Adafruit I wanted to have a custom UI streamlined for use with the tiny 
(320x200) display resolution...and since I'm usually looking for a challenge I decided to write a tiny UI library in C that 
uses SDL 1.2 & tslib (https://github.com/kergoth/tslib) to render a UI to the framebuffer.

# Why use the outdated SDL1.2 ?

The Adafruit website said that the display can only be used via the framebuffer device and also mentioned that 
SDL2 doesn't work properly.

# Why use tslib and not the touch support from SDL ?

Touch support in SDL1.2 seems to be flaky.

# How do you play music on the Raspi ?

I'm going to use the excellent mopidy (https://www.mopidy.com/) framework along with the nice python-mpd2 (https://github.com/Mic92/python-mpd2) module.

# How do you interface with Spotify?

I'm going to use the mopidy-spotify plugin 

# Requirements 

## Building the C part

You'll need the usual suspects (gcc >=5.4.0,make,ld,etc) and the following packages plus their header files:

- cmake >= 3.5.1
- libsdl 1.2.15
- libsdl-gfx 1.2-5
- libsdl-ttf2.0-0

## For the Python part

- python 2.7 (yes, I'm too lazy to learn Python 3.)
- pip install python-mpd2
- mopidy
- mopidy-spotify

# Current status

- I'm still developing the C library on my desktop only so the tslib support is not implemented yet. 
  For the time being I'm using the mouse for UI testing.
- The UI library currently only supports very crude click buttons and scrollable list views.
- Apart from a proof-of-concept I didn't start working on the Python code to integrate with mopidy

# To do

- A lot...
