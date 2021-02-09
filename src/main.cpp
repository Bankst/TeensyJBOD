#include <Arduino.h>
#include <NativeEthernet.h>
#include <TeensyID.h>

#include "SerialLogger.h"
#include "NetManager.h"
#include "WebServer.h"
#include "SdManager.h"


IPAddress ip(10, 0, 0, 2);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);

NetManager netManager;
// NetManager netManager(ip, gateway, subnet);
SdManager sdManager;
WebServer webServer;

void log_debug(std::string str) { cout << F("[main.cpp - DEBUG] ") << str.c_str() << endl; }
void log_info(std::string str) { cout << F("[main.cpp - INFO] ") << str.c_str() << endl; }
void log_warn(std::string str) { cout << F("[main.cpp - WARNING] ") << str.c_str() << endl; }
void log_error(std::string str) { cout << F("[main.cpp - ERROR] ") << str.c_str() << endl; }

void setup()
{
  Serial.begin(115200);

  // wait up to 5sec for serial connection
  uint32_t serial_start_millis = millis();
  while (!Serial && millis() - serial_start_millis < 5000) {
    ;
  }

  log_info("TeensyJBOD v0.1.10 starting");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);

  if (netManager.begin())
  {
    if (sdManager.begin())
    {
      sdManager.print_info(false);

      webServer = WebServer(80, sdManager.get_fs());
    }
    else
    {
      log_warn("No SD card!");
    }

    // start the server
    webServer.init();
    

  } else {
    log_error("Net init failed!");
  }

  // fan control
}

void loop()
{
  webServer.service();
}
