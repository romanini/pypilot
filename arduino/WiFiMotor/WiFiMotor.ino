
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

int keyIndex = 0;            // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(23);
WiFiClient client;

#define BUF_SIZE 100
#define MAX_MOTOR_PLUS 255
#define MAX_MOTOR_NEG 255
#define MOTOR_PLUS_PIN 2
#define MOTOR_NEG_PIN 3
// TODO Make sure these are correct directions once on the boat.
#define DIRECTION_POSITIVE 'Port'
#define DIRECTION_NEGATIVE 'Starbord'

char command_buffer[BUF_SIZE];
int cmd_count = BUF_SIZE;

int time_mot=0;
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

int process_cmd(char buffer[]) {
  if (buffer[0] == 't') {
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
    return 0;
  } else {
    client.println("-1 Command not understood");
    return -1;
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
        command_buffer[BUF_SIZE-cmd_count]=0;
        process_cmd(command_buffer);
        cmd_count = BUF_SIZE;
      } else {
        if (cmd_count > 0) {
          command_buffer[BUF_SIZE-cmd_count]=thisChar;
          cmd_count--;
        }
      }
    }
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
