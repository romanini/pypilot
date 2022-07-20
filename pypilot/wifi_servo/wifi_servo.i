/* File: wifi_servo.i */
%module wifi_servo

%{
#include "wifi_servo.h"
%}

class WifiServo
{

public:
    enum CommandResult {OK, COMMAND_NOT_SENT, NOT_CONNECTED, UNKNOWN };
    WifiServo();

    WifiServo::CommandResult timeCommand(double relativeTime);
    bool fault();

    int flags;
};
