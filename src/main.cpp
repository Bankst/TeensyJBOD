#include <Arduino.h>
#include <TeensyThreads.h>
#include <NativeEthernet.h>
#include <TeensyID.h>

#include <set>
#include <iostream>

ThreadWrap(Serial, SerialX);
#define Serial ThreadClone(SerialX)

#include "SerialLogger.h"
#include "NetManager.h"
#include "WebServer.h"
#include "SdManager.h"
#include "I2CManager.h"
#include "RestApi.h"
#include "TaskManager.h"

IPAddress ip(172, 16, 253, 69);
IPAddress gateway(172, 16, 0, 1);
IPAddress subnet(255, 255, 0, 0);

// NetManager net_manager;
NetManager net_manager{ip, gateway, subnet};
SdManager sd_manager;
WebServer web_server;
I2CManager i2c_manager;
RestApi restApi{&web_server, &sd_manager};
TaskManager task_manager;

void log_debug(std::string str) { serialout << F("[main.cpp - DEBUG] ") << str.c_str() << endl; }
void log_info(std::string str) { serialout << F("[main.cpp - INFO] ") << str.c_str() << endl; }
void log_warn(std::string str) { serialout << F("[main.cpp - WARNING] ") << str.c_str() << endl; }
void log_error(std::string str) { serialout << F("[main.cpp - ERROR] ") << str.c_str() << endl; }

const int status_led_full_period = 1000;
const int status_led_on_period = 100;
const int status_led_off_period = status_led_on_period / 2;

int status_led_blinks = 2; // 1 indicates normal operation, 2 indicates error

void status_led_loop()
{
  while (1)
  {
    int end_delay = status_led_full_period - ((status_led_on_period + status_led_off_period) * status_led_blinks);
    for (int i = 0; i < status_led_blinks; i++)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      threads.delay(status_led_on_period);
      digitalWrite(LED_BUILTIN, LOW);
      threads.delay(status_led_off_period);
    }
    threads.delay(end_delay);
  }
}

void task_loop()
{
  while (1)
  {
    task_manager.service();
  }
}

void web_loop()
{
  while (1)
  {
    web_server.service();
  }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  // wait up to 5sec for serial connection
  uint32_t serial_start_millis = millis();
  while (!Serial && millis() - serial_start_millis < 5000)
  {
    delay(50);
  }

  log_info("TeensyJBOD v0.2.7 starting");

  if (net_manager.begin())
  {
    // if (sdManager.begin())
    // {
    //   sdManager.print_info(false);
    // }

    // start the server
    web_server.init();
    threads.addThread(web_loop, 0, 2560, 0);
  }
  else
  {
    log_error("Net init failed!");
    status_led_blinks = 3;
  }

  task_manager.add_task(TaskManager::TimedTask{[](){i2c_manager.fan_speed_scan_task();}, 10000});
  // task_manager.add_task(TaskManager::TimedTask{[](){i2c_manager.i2c_scan_task();}, 5000});

  threads.addThread(task_loop, 0, 2560, 0);

  i2c_manager.begin_fan_control();

  // always do this last
  threads.addThread(status_led_loop, 0, 128, 0);
}

void loop() {}
