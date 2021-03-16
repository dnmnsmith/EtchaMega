#pragma once

#include <Arduino.h>


void initPositionDisplay();


// axis is 'X' 'Y' or 'Z'
// s is expected to be +-0.000 form number

void displayAxis( uint8_t axis, const char * s);

void processBuffer( uint8_t* u );

