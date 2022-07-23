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

    WifiServo::CommandResult wheel(double relativeTime);
    WifiServo::CommandResult heading(double heading);
    WifiServo::CommandResult track(double track);
    WifiServo::CommandResult mode(char *mode);
    WifiServo::CommandResult endabled(int enabled);
    bool fault();

    int flags;
};
