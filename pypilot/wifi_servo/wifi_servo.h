/* Copyright (C) 2021 Marco Romanini <romaninim@gmail.com>
 *
 * This Program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 */

class WifiServo
{

public:
    WifiServo();

    int command(double command);
    bool fault();

    int flags;


};
