
/*
 Chat  Server

 A simple server that distributes any incoming messages to all
 connected clients.  To use, telnet to your device's IP address and type.
 You can see the client's input in the serial monitor as well.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the WiFi.begin() call accordingly.

 Circuit:
 * Board with NINA module (Arduino MKR WiFi 1010, MKR VIDOR 4000 and UNO WiFi Rev.2)

 created 18 Dec 2009
 by David A. Mellis
 modified 31 May 2012
 by Tom Igoe

 */

#include <SPI.h>
#include <WiFiNINA.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
IPAddress ip(10, 10, 10, 100);

int status = WL_IDLE_STATUS;

WiFiServer debug_server(23);
WiFiServer command_server(8023);
// WiFiClient client;

#define BUF_SIZE 100
#define MAX_MOTOR_PLUS 255
#define MAX_MOTOR_NEG 255
#define MOTOR_PLUS_PIN 2
#define MOTOR_NEG_PIN 3
// TODO Make sure these are correct directions once on the boat.
#define DIRECTION_POSITIVE "port"
#define DIRECTION_NEGATIVE "starbord"

char command_buffer[BUF_SIZE];
int command_count = BUF_SIZE;
char debug_buffer[BUF_SIZE];
int debug_count = BUF_SIZE;

int time_mot=0;
float heading = 0;
float track = 0;
char mode[BUF_SIZE] = { 0 };
int enabled = 0;

int track_adjust = 0;
char mode_adjust[BUF_SIZE] = { 0 };
int enabled_adjust = -1;

int dir;
boolean command_client_already_connected = false; // whether or not the client was connected previously
boolean debug_client_already_connected = false; // whether or not the client was connected previously

void setup() {

  pinMode(MOTOR_PLUS_PIN,OUTPUT);
  pinMode(MOTOR_NEG_PIN,OUTPUT);
  analogWrite(MOTOR_PLUS_PIN,0);
  analogWrite(MOTOR_NEG_PIN,0);
  //Initialize serial and wait for port to open:
  Serial.begin(38400);
  // while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  WiFi.config(ip);

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }

  // start the servers:
  debug_server.begin();
  command_server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();

}

void process_wheel(WiFiClient client, char buffer[]) {
  float value = atof(&buffer[1]);
  int run_mills = value * 1000;
  analogWrite(MOTOR_PLUS_PIN,0);
  analogWrite(MOTOR_NEG_PIN,0);
  Serial.print("Turning wheel to ");
  if (run_mills > 0) {
    analogWrite(MOTOR_PLUS_PIN,MAX_MOTOR_PLUS);
    Serial.print(DIRECTION_POSITIVE);
  } else if (run_mills < 0) {
    analogWrite(MOTOR_NEG_PIN,MAX_MOTOR_NEG);
    Serial.print(DIRECTION_NEGATIVE);
    run_mills*=-1;
  }
  Serial.print(" for ");
  Serial.print(run_mills);
  Serial.println(" ms");
  unsigned int cur_mills = millis();
  time_mot = cur_mills + run_mills;
  client.println("ok");
}

void process_heading(WiFiClient client, char buffer[]) {
  heading = atof(&buffer[1]);
  Serial.print("Heading is ");
  Serial.println(heading);
  client.println("ok");
}

void process_track(WiFiClient client, char buffer[]) {
  track = atof(&buffer[1]);
  Serial.print("Track is ");
  Serial.println(track);
  if (track_adjust == 0) {
    client.println("ok");
  } else {
    client.print("t");
    client.println(track_adjust);
    Serial.print("                Responsding with track adjust [");
    Serial.print(track_adjust);
    Serial.println("]");
    track_adjust = 0;
  }
}

void process_mode(WiFiClient client, char buffer[]) {
  strcpy(mode,&buffer[1]);
  mode[strlen(buffer)] = '\0';
  Serial.print("Mode is ");
  Serial.println(mode);
  if (strlen(mode_adjust) == 0) {
    client.println("ok");
  } else {
    client.print("m");
    client.println(mode_adjust);
    Serial.print("                      Responsding with mode adjust [");
    Serial.print(mode_adjust);
    Serial.println("]");
    mode_adjust[0] = '\0';   
  }
}

void process_enabled(WiFiClient client, char buffer[]) {
  enabled = atoi(&buffer[1]);
  Serial.print("Enabled is ");
  Serial.println(enabled);
  if (enabled_adjust == -1) {
    client.println("ok");
  } else {
    client.print("e");
    client.println(enabled_adjust);
    Serial.print("                       Responsding with enabled adjust [");
    Serial.print(enabled_adjust);
    Serial.println("]");
    enabled_adjust = -1;
  }
}

void process_display(WiFiClient client) {
  Serial.println("Displaying info");
  client.print("Heading: ");
  client.println(heading);
  client.print("Track: ");
  client.print(track);
  client.print(" adjust by ");
  client.println(track_adjust);
  client.print("Mode: ");
  client.println(mode);
  client.print(" adjust by ");
  client.println(mode_adjust);
  client.print("Enabled: ");
  client.print(enabled);
  client.print(" adjust by ");
  client.println(enabled_adjust);
}

void process_adjust_track(WiFiClient client, char buffer[]) {
  track_adjust += atof(&buffer[2]);
  Serial.print("                          Track adjust is ");
  Serial.println(track_adjust);
  client.println("ok");
}

void process_adjust_mode(WiFiClient client, char buffer[]) {
  strcpy(mode_adjust,&buffer[2]);
  mode_adjust[strlen(buffer) - 1] = '\0';
  Serial.print("                          Mode adjust is [");
  Serial.print(mode_adjust);
  Serial.println("]");
  client.println("ok");
}

void process_adjust_enabled(WiFiClient client, char buffer[]) {
  enabled_adjust = atoi(&buffer[2]);
  Serial.print("                          Enabled adjust is ");
  Serial.println(enabled_adjust);
  client.println("ok");
}

void process_help(WiFiClient client) {
  client.println("Possible commands:");
  client.println("");
  client.println("\tae<0|1> \t\t- Adjust enalbed.  0 = disabled and 1 = enabled.");
  client.println("\tam<gps|compass> \t- Adjust the mode to the specified mode.");
  client.println("\tat<track offset> \t- Adjust track to be <track offset> from current heading.");
  client.println("\td \t\t\t- Display current information such as enabled, track, mode, heading etc.");
  client.println("\te<0|1> \t\t\t- Set enabled to be the specified value.  0 = disabled and  1 = enabled.");
  client.println("\th<heding> \t\t- Set the current heading to <heading>");
  client.println("\tm<gps|comapss> \t\t- Set the current mode to the specified mode.");
  client.println("\tt<track> \t\t- Set the current to <track>.");
  client.println("\tw<time in seconds> \t- Start rotatig the wheel for <time in seconds>.");
  client.println("\t? \t\t\t- Print this help screen");
}

void process_command(WiFiClient client, char buffer[]) {
  char command = buffer[0];
  switch (command) {
    case 'a':
      process_adjust_command(client, buffer);
      break;
    case 'd':
      process_display(client);
      break;
    case 'e':
      process_enabled(client, buffer);
      break;
    case 'h':
      process_heading(client, buffer);
      break;
    case 'm':
      process_mode(client, buffer);
      break;
    case 't':
      process_track(client, buffer);
      break;
    case 'w':
      process_wheel(client, buffer);
      break;
    case '?':
      process_help(client);
      break;
    default:
      client.println("-1 Command not understood");
      break;
  }
}

void process_adjust_command(WiFiClient client, char buffer[]){
  char sub_command = buffer[1];
  switch (sub_command) {
    case 'e':
      process_adjust_enabled(client, buffer);
      break;   
    case 'm':
      process_adjust_mode(client, buffer);
      break; 
    case 't':
      process_adjust_track(client, buffer);
      break;
    default:
      Serial.println("-1 Adjust sub-Command not understood");
      break;
  }  
}
void read_command(WiFiClient client) {
  // when the client sends the first byte, say hello:
  if (client) {
    if (client.available() > 0) {
      // read the bytes incoming from the client:
      char thisChar = client.read();
      if (thisChar == '\n') {
        command_buffer[BUF_SIZE - command_count]=0;
        process_command(client, command_buffer);
        command_count = BUF_SIZE;
      } else {
        if (command_count > 0) {
          command_buffer[BUF_SIZE - command_count]=thisChar;
          command_count--;
        }
      }
    }
  }
}

void read_debug(WiFiClient client) {
  // when the client sends the first byte, say hello:
  if (client) {
    if (client.available() > 0) {
      // read the bytes incoming from the client:
      char thisChar = client.read();
      if (thisChar == '\n') {
        debug_buffer[BUF_SIZE - debug_count]=0;
        process_command(client, debug_buffer);
        debug_count = BUF_SIZE;
      } else {
        if (debug_count > 0) {
          debug_buffer[BUF_SIZE - debug_count]=thisChar;
          debug_count--;
        }
      }
    }
  }
}
void loop() {
  unsigned int cur_mills = millis();
  if(time_mot < cur_mills && time_mot) {
    time_mot = 0;
    Serial.println("Stopping motor");
    analogWrite(MOTOR_PLUS_PIN,0);
    analogWrite(MOTOR_NEG_PIN,0);
  }

  // wait for a new client on command server:
  WiFiClient command_client = command_server.available();
  if (command_client) {  
    if (!command_client_already_connected) {
      // clear out the input buffer:
      Serial.println("We have a new command client");
      command_client.flush();
      command_client_already_connected = true;
    }
    read_command(command_client);
  } 

  // wait for a new client on debug server:
  WiFiClient debug_client = debug_server.available();
  if (debug_client) {
    if (!debug_client_already_connected) {
      // clear out the input buffer:
      Serial.println("We have a new debug client");
      debug_client.flush();
      debug_client_already_connected = true;
    }
    read_debug(debug_client);
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
