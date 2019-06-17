#!/bin/bash
/bin/gcc9/linux/bin/avr-g++ -std=c++2a -Os -mmcu=atmega328p -save-temps -DF_CPU=16000000UL -fconcepts -Wall -Wextra -Wshadow -Wfloat-conversion -Wsign-conversion -Wsign-compare -Wmissing-field-initializers -fno-threadsafe-statics  -fverbose-asm -S ../main.cpp