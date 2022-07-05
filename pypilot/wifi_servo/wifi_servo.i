/* File: wifi_servo.i */
%module wifi_servo

%{
#include "wifi_servo.h"
%}

class WifiServo
{

public:
    WifiServo();

    void command(double command);
    bool fault();

    int flags;
};
