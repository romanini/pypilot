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
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>

#include "wifi_servo.h"

#define PORT 23
#define IP "10.10.10.100"
// #define IP "172.16.0.90"
#define EOL "\n"

WifiServo::WifiServo() 
{
    this->flags = 0;
    this->isConnected = false;

    if ((this->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
    }
    this->address.sin_family = AF_INET;
    this->address.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, IP, &(this->address.sin_addr)) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
    }
}

WifiServo::CommandResult WifiServo::wheel(double relativeTime) {
    char command[100];
    sprintf(command, "w%f",relativeTime);
    return this->sendCommand(command);
}

WifiServo::CommandResult WifiServo::heading(double heading) {
    char command[100];
    sprintf(command, "h%f",heading);
    return this->sendCommand(command);
}

WifiServo::CommandResult WifiServo::track(double track) {
    char command[100];
    sprintf(command, "t%f",track);
    return this->sendCommand(command);
}

WifiServo::CommandResult WifiServo::mode(char *mode) {
    char command[100];
    sprintf(command, "m%s",mode);
    return this->sendCommand(command);
}

WifiServo::CommandResult WifiServo::enabled(int enabled) {
    char command[100];
    sprintf(command, "e%d",enabled);
    return this->sendCommand(command);

}

WifiServo::CommandResult WifiServo::sendCommand(char *command) {
    if (this->connect()) {
        size_t sendLen = send(this->sock, command, strlen(command), 0);
        if (sendLen != strlen(command)) {
            printf("\nDid not send full command [%s], only sent [%d] bytes", command, sendLen);
            return COMMAND_NOT_SENT;
        }
        // send EOL so teh arduino knows command is finished.
        char buffer[256] = { 0 };
        send(this->sock, EOL, strlen(EOL), 0);
        read(this->sock, buffer, 256);
        if (strcmp(buffer, "ok\n")) {
            return OK;
        } else {
            printf("Got unexpected error: [%s]", buffer);
            return UNKNOWN;
        }
    }
    return NOT_CONNECTED;
}

bool WifiServo::fault() {
    return flags;
}

void WifiServo::disconnect() {
    // disonnect from arduino
    if (this->isConnected) {
        close(this->client_fd);
        this->isConnected = false;
    }
}

bool WifiServo::connect() {
    if (!this->isConnected) {
        if ((this->client_fd = ::connect(this->sock, (struct sockaddr*)&(this->address), sizeof(this->address))) < 0) {
            printf("\nConnection Failed [%d]\n", errno );
        } else {
            this->isConnected = true;
    	}
    }
    return this->isConnected;
}


