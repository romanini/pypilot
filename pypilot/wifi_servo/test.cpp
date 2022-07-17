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
    char cmd1[] = "t0.5";
    WifiServo::CommandResult ret = servo.command(cmd1);
    printf("Sent [%s] and got back [%d]", cmd1, ret);
    char cmd2[] = "t0.6";
    ret = servo.command(cmd2);
    printf("Sent [%s] and got back [%d]", cmd2, ret);
}
