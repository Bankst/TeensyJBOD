#include <Arduino.h>
#include <NativeEthernet.h>
#include <TeensyID.h>

#include "SerialLogger.h"
#include "NetManager.h"
#include "WebServer.h"
#include "SdManager.h"

uint8_t mac[6];

SdManager sdManager;
WebServer webServer;

void log_debug(std::string str) { cout << F("[main.cpp - DEBUG] ") << str.c_str() << endl; }
void log_info(std::string str) { cout << F("[main.cpp - INFO] ") << str.c_str() << endl; }
void log_warn(std::string str) { cout << F("[main.cpp - WARNING] ") << str.c_str() << endl; }
void log_error(std::string str) { cout << F("[main.cpp - ERROR] ") << str.c_str() << endl; }

void initSerial()
{
  Serial.begin(115200);
  log_info("Serial init at 115200 baud");
}

void initEthernet()
{
  // get MAC address
  teensyMAC(mac);
  Serial.printf("[main.cpp - INFO] MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // start the Ethernet connection
  Ethernet.begin(mac);

  if (Ethernet.linkStatus() == LinkOFF)
  {
    log_info("Ethernet link DOWN");
  }
  else if (Ethernet.linkStatus() == LinkON)
  {
    log_info("Ethernet link UP");
  }
}

void setup()
{
  initSerial();
  log_info("TeensyJBOD v0.1.5 starting");
  initEthernet();

  pinMode(LED_BUILTIN, OUTPUT);

  if (!sdManager.init())
  {
    log_warn("No SD card!");
  }
  else
  {
    sdManager.print_info(false);

    webServer = WebServer(80, sdManager.get_fs());
  }

  // start the server
  webServer.init();
  Serial.print("[main.cpp - INFO] Server is at ");
  Serial.println(Ethernet.localIP());

  // fan control
}

void loop()
{
  webServer.service();
}
