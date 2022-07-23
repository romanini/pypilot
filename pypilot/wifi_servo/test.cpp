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
    WifiServo::CommandResult ret = servo.wheel(rt1);
    printf("Sent wheel [%f] and got back [%d]\n", rt1, ret);
    double rt2 = 0.6;
    ret = servo.wheel(rt2);
    printf("Sent wheel [%f] and got back [%d]\n", rt2, ret);
    double h = 123.4;
    ret = servo.heading(h);
    printf("Sent heading [%f] and got back [%d]\n", h, ret);
    double t = 321.4;
    ret = servo.track(t);
    printf("Sent track [%f] and got back [%d]\n", t, ret);
    char m[] = "compass";
    ret = servo.mode(m);
    printf("Sent mode [%s] and got back [%d]\n", m, ret);
    int e = 1;
    ret = servo.enabled(e);
    printf("Sent enabled [%d] and got back [%d]\n", e, ret);
}
