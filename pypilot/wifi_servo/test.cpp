#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "wifi_servo.h"

int main(void)
{
    printf("starting...");
    WifiServo servo;
    double rt1 = 0.5;
    WifiServo::CommandResult ret = servo.timeCommand(rt1);
    printf("Sent [%d] and got back [%d]", rt1, ret);
    double rt2 = 0.6;
    ret = servo.timeCommand(rt2);
    printf("Sent [%d] and got back [%d]", rt2, ret);
}
