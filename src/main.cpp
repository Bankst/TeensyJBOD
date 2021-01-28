#include <Arduino.h>
#include <NativeEthernet.h>
#include <TeensyID.h>
#include <TeensyThreads.h>
// #include "WebServer.h"

uint8_t mac[6];

// WebServer webServer(80, "");

void initSerial() {
  Serial.begin(115200);
  Serial.println("Serial INIT - 115200");
}

void initEthernet() {
  // get MAC address
  teensyMAC(mac);
  Serial.printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // start the Ethernet connection
  Ethernet.begin(mac);
  
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet link DOWN");
  } else if (Ethernet.linkStatus() == LinkON) {
    Serial.println("Ethernet link UP");
  }
}

void setup()
{
  initSerial();
  Serial.println("TeensyJBOD v0.1 starting");
  initEthernet();

  pinMode(LED_BUILTIN,OUTPUT);

  // start the server
  // webServer.init();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
}

void loop()
{
  // webServer.service();
}
