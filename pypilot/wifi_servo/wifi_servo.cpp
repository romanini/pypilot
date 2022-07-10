/* Copyright (C) 2019 Sean D'Epagnier <seandepagnier@gmail.com>
 *
 * This Program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 */

#include <stdint.h>
#include <unistd.h>
#include <math.h>

#include <stdio.h>
#include <errno.h>

#include <sys/time.h>

#include "wifi_servo.h"

WifiServo::WifiServo()
{
    flags = 0;
}

int WifiServo::command(double command)
{    
    command = fmin(fmax(command, -1), 1);
    // send the command!
    return 0;
}

bool WifiServo::fault()
{
    return flags;
}


