#pragma once

#include <ArduinoStream.h>

ArduinoOutStream serialout(Serial);

ThreadWrap(serialout, serialoutX);
#define serialout ThreadClone(serialoutX)