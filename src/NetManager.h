#pragma once

#include <NativeEthernet.h>
#include <TeensyID.h>

#include <sstream>

#include "StringUtil.h"
#include "SerialLogger.h"

class NetManager
{
private:
    uint8_t mac[6];
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;

public:
    NetManager() : NetManager(IPAddress(), IPAddress(), IPAddress()) {};

    NetManager(IPAddress ip, IPAddress gateway, IPAddress subnet) : ip(ip), gateway(gateway), subnet(subnet)
    {
        teensyMAC(mac);
        Serial.printf(F("[NetManager.h - INFO] MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \n"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    };

    bool begin();

private:
    void log_debug(std::string str) { serialout << F("[NetManager.h - DEBUG] ") << str.c_str() << endl; }
    void log_info(std::string str) { serialout << F("[NetManager.h - INFO] ") << str.c_str() << endl; }
    void log_warn(std::string str) { serialout << F("[NetManager.h - WARNING] ") << str.c_str() << endl; }
    void log_error(std::string str) { serialout << F("[NetManager.h - ERROR] ") << str.c_str() << endl; }
};

bool NetManager::begin()
{
    bool is_static = ip != 0 && gateway != 0 && subnet != 0;

    std::string mode_str = (is_static ? "Static" : "DHCP");
    log_info("Bringing up ethernet - " + mode_str);


    uint32_t eth_up_start_millis = millis();

    if (is_static) {
        Ethernet.begin(mac, ip, gateway, gateway, subnet);
    } else if (!Ethernet.begin(mac, 10000, 5000)) {
        // TODO: default to a known static IP upon dhcp failure? retry forever??
        return false;
    }

    
    if (Ethernet.linkStatus() == LinkOFF)
    {
        log_info("Ethernet PHY link DOWN");
    }
    else if (Ethernet.linkStatus() == LinkON)
    {
        log_info("Ethernet PHY link UP: " + to_string(millis() - eth_up_start_millis) + " ms");
        Serial.print("[NetManager.h - INFO] IP address: ");
        Serial.println(Ethernet.localIP());
    }

    return true;
}

