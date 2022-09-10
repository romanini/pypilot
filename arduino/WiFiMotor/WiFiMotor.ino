#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS2MDL.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
IPAddress ip(10, 10, 10, 100);

int status = WL_IDLE_STATUS;

Adafruit_LIS2MDL mag = Adafruit_LIS2MDL(12345);
float bearing = 0;

WiFiServer debug_server(23);
WiFiServer command_server(8023);
// WiFiClient client;

#define BUF_SIZE 100
#define MAX_MOTOR_PLUS 255
#define MAX_MOTOR_NEG 255
#define MOTOR_PLUS_PIN 2
#define MOTOR_NEG_PIN 3
#define DIRECTION_POSITIVE "port"
#define DIRECTION_NEGATIVE "starbord"
#define COMPASS_READ_INTERVAL 20 // read at 50Hz for high accuracy
#define COMPASS_SHORT_AVERAGE_SIZE 100
#define COMPASS_LONG_AVERAGE_SIZE 1000

char command_buffer[BUF_SIZE];
int command_count = BUF_SIZE;
char debug_buffer[BUF_SIZE];
int debug_count = BUF_SIZE;

int motor_stop_time_mills=0;
float short_average_heading = 0;
float long_average_heading = 0;
float short_average_heading_change = 0;
float long_average_heading_change = 0;
unsigned int last_compass_read = 0;
float heading = 0;
char mode[BUF_SIZE] = { 0 };
int enabled = 0;

float heading_adjust;
char mode_adjust[BUF_SIZE] = { 0 };
int enabled_adjust = -1;

boolean command_client_already_connected = false; // whether or not the client was connected previously
boolean debug_client_already_connected = false; // whether or not the client was connected previously

void setup() {
  pinMode(MOTOR_PLUS_PIN,OUTPUT);
  pinMode(MOTOR_NEG_PIN,OUTPUT);
  analogWrite(MOTOR_PLUS_PIN,0);
  analogWrite(MOTOR_NEG_PIN,0);
  //Initialize serial 
  Serial.begin(38400);

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

  mag.setDataRate(LIS2MDL_RATE_50_HZ);

  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the LIS2MDL ... check your connections */
    Serial.println("Ooops, no LIS2MDL detected ... Check your wiring!");
    while(1);
  }

  // start the servers:
  debug_server.begin();
  command_server.begin();
  // you're connected now, so print out the status:
  print_wifi_status();
}

void print_wifi_status() {
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

void process_wheel(WiFiClient client, char buffer[]) {
  float value = atof(&buffer[1]);
  int run_mills = value * 1000;
  start_motor(run_mills);
  client.println("ok");
}

void process_bearing(WiFiClient client, char buffer[]) {
  bearing = atof(&buffer[1]);
  Serial.print("Bearing is ");
  Serial.println(bearing);
  client.println("ok");
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
  client.print("Bearing: ");
  client.println(bearing);
  client.print("Heading: ");
  client.print(heading);
  client.print(" adjust by ");
  client.println(heading_adjust);
  client.print("Mode: ");
  client.println(mode);
  client.print(" adjust by ");
  client.println(mode_adjust);
  client.print("Enabled: ");
  client.print(enabled);
  client.print(" adjust by ");
  client.println(enabled_adjust);
}

void process_adjust_heading(WiFiClient client, char buffer[]) {
  heading_adjust += atof(&buffer[2]);
  Serial.print("                          Heading adjust is ");
  Serial.println(heading_adjust);
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
  client.println("\tah<heading offset> \t- Adjust heading to be <heading offset> from current heading.");
  client.println("\td \t\t\t- Display current information such as enabled, mode, heading etc.");
  client.println("\te<0|1> \t\t\t- Set enabled to be the specified value.  0 = disabled and  1 = enabled.");
  client.println("\tm<gps|comapss> \t\t- Set the current mode to the specified mode.");
  client.println("\tb<bearing> \t\t- Set the current to <bearing>.");
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
    case 'm':
      process_mode(client, buffer);
      break;
    case 'b':
      process_bearing(client, buffer);
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
    case 'h':
      process_adjust_heading(client, buffer);
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

void start_motor(int run_mills) {
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
  motor_stop_time_mills = cur_mills + run_mills;
}
/*
 * See if the motor is running and needs to stop
 */
void check_motor() {
  unsigned int cur_mills = millis();
  if(motor_stop_time_mills < cur_mills && motor_stop_time_mills) {
    motor_stop_time_mills = 0;
    Serial.println("Stopping motor");
    analogWrite(MOTOR_PLUS_PIN,0);
    analogWrite(MOTOR_NEG_PIN,0);
  }
}

float read_compass() {
  /* Get a new sensor event */
  sensors_event_t event;
  mag.getEvent(&event);
  // Calculate the angle of the vector y,x to get the heading
  float heading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / PI;
  
  // Normalize heading to 0-360
  if (heading < 0){
    heading = 360 + heading;
  }
  return heading;  
}

/*
 * See if it is time to read the compass, if so adjust the running averages and if necessary the heading adjust;
 */
void check_compass() {
  unsigned int cur_mills = millis();
  if (last_compass_read - cur_mills > COMPASS_READ_INTERVAL) {
    float heading = read_compass();
    long_average_heading = long_average_heading + ((heading - long_average_heading) / COMPASS_LONG_AVERAGE_SIZE);
    short_average_heading = short_average_heading + ((heading - short_average_heading) / COMPASS_SHORT_AVERAGE_SIZE);
    long_average_heading_change = long_average_heading_change + (((heading - long_average_heading) - long_average_heading_change) / COMPASS_LONG_AVERAGE_SIZE);
    short_average_heading_change = short_average_heading_change + (((heading - short_average_heading) - short_average_heading_change) / COMPASS_SHORT_AVERAGE_SIZE);
    if (strcmp(mode,"compass") == 0) {
      float alter = bearing - short_average_heading;
      if (alter > 180) {
        alter =  alter - 360;
      } else if (alter < -180) {
        alter = alter + 360;
      }
      heading_adjust = alter;
    }
  }
}
/*
 * For now we just use short average heading and don't use heading change, but this is where we would make use
 * of all those to make more intelligent course corrections. 
 */
void check_heading() {
  if (short_average_heading > 1.0 or short_average_heading < -1.0) {
    // 200 is a complete guess, we'll play with this a bunch and make a constant out of it.
    int run_mills = short_average_heading * 200;
    start_motor(run_mills);
  }
}

void loop() {

  check_motor();
  check_compass();
  check_heading();

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


