#pragma once

// #include <Arduino.h>
#include <NativeEthernet.h>
#include <TeensyID.h>

#include <sstream>
#include <iostream>

#include "StringUtil.h"
#include "SerialLogger.h"



class NetManager
{
private:
    uint8_t mac[6];

public:
    NetManager();
    // TODO: static IP
    // NetManager(IPAddress static_ip, IPAddress netmask, IPAddress gateway);

    bool begin();

private:
    void log_debug(std::string str) { cout << F("[NetManager.h - DEBUG] ") << str.c_str() << endl; }
    void log_info(std::string str) { cout << F("[NetManager.h - INFO] ") << str.c_str() << endl; }
    void log_warn(std::string str) { cout << F("[NetManager.h - WARNING] ") << str.c_str() << endl; }
    void log_error(std::string str) { cout << F("[NetManager.h - ERROR] ") << str.c_str() << endl; }
};

NetManager::NetManager()
{
    teensyMAC(mac);
    Serial.printf("[main.cpp - INFO] MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

bool NetManager::begin()
{
    log_info("Bringing up ethernet...");

    uint32_t eth_up_start_millis = millis();

    if (!Ethernet.begin(mac, 25000, 5000)) {
        return false;
    }

    if (Ethernet.linkStatus() == LinkOFF)
    {
        log_info("Ethernet PHY link DOWN");
    }
    else if (Ethernet.linkStatus() == LinkON)
    {
        std::ostringstream ss;
        uint32_t eth_up_millis = millis() - eth_up_start_millis;
        ss << eth_up_millis;
        log_info("Ethernet PHY link UP: " + ss.str() + " ms");
    }

    return true;
}

