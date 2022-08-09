
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

WiFiServer server(23);
WiFiClient client;

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
char serial_command_buffer[BUF_SIZE];
int serial_command_count = BUF_SIZE;

int time_mot=0;
float heading = 0;
float track = 0;
char mode[BUF_SIZE] = "";
int enabled = 0;

int track_adjust = 0;
char mode_adjust[BUF_SIZE] = "";
int enabled_adjust = -1;

int dir;
boolean alreadyConnected = false; // whether or not the client was connected previously

void setup() {

  pinMode(MOTOR_PLUS_PIN,OUTPUT);
  pinMode(MOTOR_NEG_PIN,OUTPUT);
  analogWrite(MOTOR_PLUS_PIN,0);
  analogWrite(MOTOR_NEG_PIN,0);
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

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

  // start the server:
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();

}

void process_wheel(char buffer[]) {
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

void process_heading(char buffer[]) {
  heading = atof(&buffer[1]);
  Serial.print("Heading is ");
  Serial.println(heading);
  client.println("ok");
}

void process_track(char buffer[]) {
  track = atof(&buffer[1]);
  Serial.print("Track is ");
  Serial.println(track);
  if (track_adjust == 0) {
    client.println("ok");
  } else {
    client.print("t");
    client.println(track_adjust);
    track_adjust = 0;
  }
}

void process_mode(char buffer[]) {
  strcpy(mode,&buffer[1]);
  Serial.print("Mode is ");
  Serial.println(mode);
  if (mode_adjust == "") {
    client.println("ok");
  } else {
    client.print("m");
    client.println(mode_adjust);
    strcpy(mode_adjust, "");    
  }
}

void process_enabled(char buffer[]) {
  enabled = atoi(&buffer[1]);
  Serial.print("Enabled is ");
  Serial.println(enabled);
  if (enabled_adjust == -1) {
    client.println("ok");
  } else {
    client.print("e");
    client.println(enabled_adjust);
    enabled_adjust = -1;
  }
}

void process_cmd(char buffer[]) {
  char command = buffer[0];
  switch (command) {
    case 'w':
      process_wheel(buffer);
      break;
    case 'h':
      process_heading(buffer);
      break;
    case 't':
      process_track(buffer);
      break;
    case 'm':
      process_mode(buffer);
      break;
    case 'e':
      process_enabled(buffer);
      break;
    default:
      client.println("-1 Command not understood");
      break;
  }
}

void read_network_command() {
  // when the client sends the first byte, say hello:
  if (client) {
    if (!alreadyConnected) {
      // clear out the input buffer:
      Serial.println("We have a new client");
      client.flush();
      alreadyConnected = true;
    }

    if (client.available() > 0) {
      // read the bytes incoming from the client:
      char thisChar = client.read();
      if (thisChar == '\n') {
        command_buffer[BUF_SIZE-command_count]=0;
        process_cmd(command_buffer);
        command_count = BUF_SIZE;
      } else {
        if (command_count > 0) {
          command_buffer[BUF_SIZE-command_count]=thisChar;
          command_count--;
        }
      }
    }
  }
}

void process_adjust_track(char buffer[]) {
  track_adjust = atof(&buffer[1]);
  Serial.print("Track adjust is ");
  Serial.println(track_adjust);
}

void process_adjust_mode(char buffer[]) {
  strcpy(mode_adjust,&buffer[1]);
  Serial.print("Mode adjust is ");
  Serial.println(mode_adjust);
}

void process_adjust_enabled(char buffer[]) {
  enabled_adjust = atoi(&buffer[1]);
  Serial.print("Enabled adjust is ");
  Serial.println(enabled_adjust);
}

void process_serial_cmd(char buffer[]) {
  char command = buffer[0];
  switch (command) {
    case 't':
      process_adjust_track(buffer);
      break;
    case 'm':
      process_adjust_mode(buffer);
      break;
    case 'e':
      process_adjust_enabled(buffer);
      break;
    default:
      Serial.println("-1 Command not understood");
      break;
  }
}

void read_serial_command() {
  // see if there is any serial input.  If there is, read it.
  while (Serial.available() > 0) {
    char thisChar = Serial.read();
    if (thisChar == '\n') {
        serial_command_buffer[BUF_SIZE - serial_command_count]=0;
        process_serial_cmd(serial_command_buffer);
        serial_command_count = BUF_SIZE;
      } else {
        if (serial_command_count > 0) {
          serial_command_buffer[BUF_SIZE - serial_command_count]=thisChar;
          serial_command_count--;
        }
      }
  }
}

void loop() {
  // wait for a new client:
  client = server.available();

  unsigned int cur_mills = millis();
  if(time_mot < cur_mills && time_mot) {
    time_mot = 0;
    Serial.println("Stopping motor");
    analogWrite(MOTOR_PLUS_PIN,0);
    analogWrite(MOTOR_NEG_PIN,0);
  }
  
  read_serial_command();
  read_network_command();
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
