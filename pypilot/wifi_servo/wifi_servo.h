/* Copyright (C) 2021 Marco Romanini <romaninim@gmail.com>
 *
 * This Program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 */

#include <arpa/inet.h>
#include <sys/socket.h>

class WifiServo
{
private:
    WifiServo::CommandResult sendCommand(char *command);
    bool connect();
    void disconnect();
    int sock;
    int client_fd;
    char *lastError;
    struct sockaddr_in address;
    bool isConnected = false;

public:
    enum CommandResult {OK, COMMAND_NOT_SENT, NOT_CONNECTED, UNKNOWN };
    WifiServo();

    WifiServo::CommandResult timeCommand(double relativeTime);
    bool fault();
    
    int flags;

};
