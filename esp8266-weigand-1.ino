/*
   Example on how to use the Wiegand reader library with interruptions.
*/

#include <Wiegand.h>
#include <string.h>
#include "FS.h"
#include <Syslog.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>

// These are the pins connected to the Wiegand D0 and D1 signals.
// Ensure your board supports external Interruptions on these pins
#define PIN_D0 D2
#define PIN_D1 D1

#define FIRMWARE_VERSION "v1"

const char* hostname = "wiegand";
const char* syslogserver = "192.168.10.21";
const int syslogport = 514;
const char* appname = "frontdoor";

const char* ssid = "junk";
const char* password = "junk";
const int wifitimeout = 10;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udpClient;

// Syslog
Syslog syslog(udpClient, SYSLOG_PROTO_IETF);


// should i reset configuration to default?
bool resetConfigToDefault = false;

bool spiffsActive = false;

String listFiles(bool ishtml = false);

// used for loading and saving configuration data
const char *filename = "/users.txt";

// log file name
const char *logfilename = "/log.txt";

const char *bit_rep[16] = {
  [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
  [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
  [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
  [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

const char *bit_hex[16] = {
  [ 0] = "0", [ 1] = "1", [ 2] = "3", [ 3] = "3",
  [ 4] = "4", [ 5] = "5", [ 6] = "6", [ 7] = "7",
  [ 8] = "8", [ 9] = "9", [10] = "A", [11] = "B",
  [12] = "C", [13] = "D", [14] = "E", [15] = "F",
};

// Add users to the list here:
String userlist = "aaaa,bbbb,cccc,dddd,eeee,ffff,CD69C86F";

// You must also add users to the list here so that we know which card matches what name
/*
  aaaa = "Steve F"
  bbbb = "Tim"
  cccc = "Steve T"
  dddd = "Alex"
  eeee = "Gary"
  ffff = "Scott"
*/

// The object that handles the wiegand protocol
Wiegand wiegand;

//void ICACHE_RAM_ATTR ISRoutine ();

// Initialize Wiegand reader
void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.print("Firmware: "); Serial.println(FIRMWARE_VERSION);

  Serial.println("Booting: ...");

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to wifi: ");
  int wifiiteration = 0;
  bool wificonnected = false;
  while (WiFi.status() != WL_CONNECTED) {
    wifiiteration++;
    delay(500);
    Serial.print(".");
    if ((wifiiteration * 500) > (wifitimeout * 1000)) {
      Serial.println("\nWifi connection attempt timeout reached, moving onwards");
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi connected");
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wifi is not currently connected");
  }

  Serial.println("Configuring Syslog ...");
  // configure syslog server using loaded configuration
  syslog.server(syslogserver, syslogport);
  syslog.deviceHostname(hostname);
  syslog.appName(appname);
  syslog.defaultPriority(LOG_INFO);

  Serial.println("Mounting SPIFFS ...");
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS_State: ERROR: Cannot mount SPIFFS");
    //return;
  } else {
    Serial.println("SPIFFS_State: Active");
    spiffsActive = true;
    Serial.print("Flash Real Size: ");
    Serial.println(ESP.getFlashChipRealSize());
    Serial.print("Flash Firmware Configured Size: ");
    Serial.println(ESP.getFlashChipSize());

    FSInfo fsInfo;
    SPIFFS.info(fsInfo);
    Serial.print("FS Bytes: ");
    Serial.println(fsInfo.usedBytes);

    Serial.println("SPIFFS Files:");
    Dir dir = SPIFFS.openDir ("");
    while (dir.next ()) {
      Serial.print(dir.fileName ()); Serial.print("  "); Serial.println(dir.fileSize ());
    }

    Serial.println("\n");
    //Serial.println("Printing logs:");
    //printLogs();
    Serial.println("Booted\n================");
  }

  if (resetConfigToDefault) {
    Serial.println("Resetting Configuration to Default");
    SPIFFS.remove(filename);
  }

  //Install listeners and initialize Wiegand reader
  wiegand.onReceive(receivedData, "Card read: ");
  wiegand.onReceiveError(receivedDataError, "Card read error: ");
  wiegand.onStateChange(stateChanged, "State changed: ");
  wiegand.begin(Wiegand::LENGTH_ANY, false);

  //initialize pins as INPUT and attaches interruptions
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_D0), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), pinStateChanged, CHANGE);

  //Sends the initial pin state to the Wiegand library
  pinStateChanged();

  syslog.log(LOG_INFO, "Booted");
}

// Every few milliseconds, check for pending messages on the wiegand reader
// This executes with interruptions disabled, since the Wiegand library is not thread-safe
void loop() {
  noInterrupts();
  wiegand.flush();
  interrupts();
  //Sleep a little -- this doesn't have to run very often.
  delay(100);
  //Serial.print("loop");
}

// When any of the pins have changed, update the state of the wiegand library
void pinStateChanged() {
  wiegand.setPin0State(digitalRead(PIN_D0));
  wiegand.setPin1State(digitalRead(PIN_D1));
}

// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char* message) {
  Serial.print(message);
  Serial.println(plugged ? "Weigand Connected" : "Weigand Disconnected");
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
void receivedData(uint8_t* data, uint8_t bits, const char* message) {
  Serial.println("===");
  Serial.print("bits="); Serial.println(bits);
  //Serial.print(message);
  //Serial.print(bits);
  //Serial.print("bits / ");
  //uint8_t bytes = (bits + 7) / 8;
  uint8_t bytes = 4;
  //Serial.print("bytes = "); Serial.println(bytes);

//=============
  char rfidstring[30] = "";
  //char * rfidstring = (char *) malloc(strlen(data)*2);
  //char rfidstring[strlen((char*) data)*2];
  char tempchar[2];
  Serial.print("length="); Serial.println(bytes);
  for (int i = 0; i < bytes; i++) {
    //Serial.print(data[i] >> 4, 16);
    //Serial.print(data[i] & 0xF, 16);
    sprintf(tempchar, "%x%x", data[i] >> 4, data[i] & 0x0F);
    strcat(rfidstring, tempchar);
    Serial.print(i); Serial.print(":"); Serial.println(rfidstring);
  }

  //rfidstring[(bytes*2)+1] = '\0';
  for (int i = 0; i < strlen(rfidstring); i++) {
    rfidstring[i] = toupper(rfidstring[i]);
  }
  
  //Serial.print("readablerfid4=");Serial.println(rfidstring);
  String foundcard = rfidstring;

  delay(500);

//===============


  //String foundcard = readablerfid4((char*) data);
  Serial.print("Card Detected: "); Serial.println(foundcard);

  syslog.logf(LOG_INFO, "Card-Detected: Card=%s", foundcard.c_str());

  if (findUser(foundcard)) {
    Serial.print(foundcard); Serial.println(": Card found, unlocking door");
    unlockDoor(foundcard);
    spiffslog(foundcard, String("Card found, unlocking door"));
  } else {
    Serial.print(foundcard); Serial.println(": Unknown Card, locking door");
    lockDoor(foundcard);
    spiffslog(foundcard, String("Unknown card, locking door"));
  }

  delete(&foundcard);
}

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
  Serial.print(message);
  Serial.print(Wiegand::DataErrorStr(error));
  Serial.print(" - Raw data: ");
  Serial.print(rawBits);
  Serial.print("bits / ");

  //Print value in HEX
  uint8_t bytes = (rawBits + 7) / 8;
  for (int i = 0; i < bytes; i++) {
    Serial.print(rawData[i] >> 4, 16);
    Serial.print(rawData[i] & 0xF, 16);
  }
  Serial.println();
}
