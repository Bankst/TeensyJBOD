#pragma once

#include <Arduino.h>
#include <SdFat.h>
#include "SerialLogger.h"

#define SD_FAT_TYPE 0
#define SD_CONFIG SdioConfig(FIFO_SDIO)
const uint8_t SD_CS_PIN = SS;

class SdManager
{
private:
    SdFat sd;

    // SD info
    cid_t m_cid;
    csd_t m_csd;
    uint32_t m_eraseSize;
    uint32_t m_ocr;

public:
    SdManager(){};
    bool begin();
    void print_info(bool detail);

    SdFs *get_fs();

private:
    std::string get_card_type();
    void print_card_type();

    void cid_dump();
    bool csd_dump();
    bool mbr_dump();
    bool vol_dump();

    bool mbr_check();

    void error_print();

    void log_debug(std::string str) { cout << F("[SdManager.h - DEBUG] ") << str.c_str() << endl; }
    void log_info(std::string str) { cout << F("[SdManager.h - INFO] ") << str.c_str() << endl; }
    void log_warn(std::string str) { cout << F("[SdManager.h - WARNING] ") << str.c_str() << endl; }
    void log_error(std::string str) { cout << F("[SdManager.h - ERROR] ") << str.c_str() << endl; }
};

bool SdManager::begin()
{
    uint32_t t = millis();
    if (!sd.cardBegin(SD_CONFIG))
    {
        log_error("SD init failed!");
        return false;
    }

    if (!sd.card()->readCID(&m_cid) ||
        !sd.card()->readCSD(&m_csd) ||
        !sd.card()->readOCR(&m_ocr))
    {
        log_error("readInfo failed");
        error_print();
        return false;
    }

    if (!mbr_check())
    {
        log_error("MBR init failed");
        return false;
    }

    if (!sd.volumeBegin())
    {
        log_error("volumeBegin failed. Is the card formatted?");
        error_print();
        return false;
    }

    t = millis() - t;
    cout << F("[SdManager.h - INFO] SD init OK: ") << t << " ms" << endl;

    return true;
}

void SdManager::print_info(bool detail)
{
    cout << F("[SdManager.h]: Card info - ");

    if (detail)
    {
        cout << endl;
        print_card_type();
        cid_dump();
        if (csd_dump())
        {
            if (mbr_dump())
            {
                vol_dump();
            }
        }
    }
    else
    {
        double capacity = 0.000512 * sdCardCapacity(&m_csd);
        cout << capacity << " MB"
             << " " << get_card_type().c_str() << endl;
    }
}

SdFs *SdManager::get_fs()
{
    return &sd;
}

std::string SdManager::get_card_type()
{
    switch (sd.card()->type())
    {
    case SD_CARD_TYPE_SD1:
        return "SD1";
    case SD_CARD_TYPE_SD2:
        return "SD2";
    case SD_CARD_TYPE_SDHC:
        return (sdCardCapacity(&m_csd) < 70000000) ? "SDHC" : "SDXC";
    default:
        return "Unknown";
    }
}

void SdManager::print_card_type()
{
    cout << F("\tCard type: ");

    switch (sd.card()->type())
    {
    case SD_CARD_TYPE_SD1:
        cout << F("SD1\n");
        break;

    case SD_CARD_TYPE_SD2:
        cout << F("SD2\n");
        break;

    case SD_CARD_TYPE_SDHC:
        if (sdCardCapacity(&m_csd) < 70000000)
        {
            cout << F("SDHC\n");
        }
        else
        {
            cout << F("SDXC\n");
        }
        break;

    default:
        cout << F("Unknown\n");
    }
}

void SdManager::cid_dump()
{
    cout << '\t' << F("Manufacturer ID: ");
    cout << uppercase << showbase << hex << int(m_cid.mid) << dec << endl;
    cout << '\t' << F("OEM ID: ") << m_cid.oid[0] << m_cid.oid[1] << endl;
    cout << '\t' << F("Product: ");
    for (uint8_t i = 0; i < 5; i++)
    {
        cout << m_cid.pnm[i];
    }
    cout << endl;
    cout << '\t' << F("Version: ");
    cout << int(m_cid.prv_n) << '.' << int(m_cid.prv_m) << endl;
    cout << '\t' << F("Serial number: ") << hex << m_cid.psn << dec << endl;
    cout << '\t' << F("Manufacturing date: ");
    cout << int(m_cid.mdt_month) << '/';
    cout << (2000 + m_cid.mdt_year_low + 10 * m_cid.mdt_year_high) << endl;
}

bool SdManager::csd_dump()
{
    bool eraseSingleBlock;
    if (m_csd.v1.csd_ver == 0)
    {
        eraseSingleBlock = m_csd.v1.erase_blk_en;
        m_eraseSize = (m_csd.v1.sector_size_high << 1) | m_csd.v1.sector_size_low;
    }
    else if (m_csd.v2.csd_ver == 1)
    {
        eraseSingleBlock = m_csd.v2.erase_blk_en;
        m_eraseSize = (m_csd.v2.sector_size_high << 1) | m_csd.v2.sector_size_low;
    }
    else
    {
        cout << F("m_csd version error\n");
        return false;
    }
    m_eraseSize++;
    cout << '\t' << F("Capacity: ") << 0.000512 * sdCardCapacity(&m_csd);
    cout << F(" MB (MB = 1,000,000 bytes)\n");

    cout << '\t' << F("flashEraseSize: ") << int(m_eraseSize) << F(" blocks\n");
    cout << '\t' << F("eraseSingleBlock: ");
    if (eraseSingleBlock)
    {
        cout << F("true\n");
    }
    else
    {
        cout << F("false\n");
    }
    return true;
}

bool SdManager::mbr_dump()
{
    MbrSector_t mbr;
    bool valid = true;
    if (!sd.card()->readSector(0, (uint8_t *)&mbr))
    {
        cout << F("\nread MBR failed.\n");
        error_print();
        return false;
    }
    cout << F("\n\tSD Partition Table\n");
    cout << F("\tpart,boot,bgnCHS[3],type,endCHS[3],start,length\n");
    for (uint8_t ip = 1; ip < 5; ip++)
    {
        MbrPart_t *pt = &mbr.part[ip - 1];
        if ((pt->boot != 0 && pt->boot != 0X80) ||
            getLe32(pt->relativeSectors) > sdCardCapacity(&m_csd))
        {
            valid = false;
        }
        cout << '\t' << int(ip) << ',' << uppercase << showbase << hex;
        cout << int(pt->boot) << ',';
        for (int i = 0; i < 3; i++)
        {
            cout << int(pt->beginCHS[i]) << ',';
        }
        cout << int(pt->type) << ',';
        for (int i = 0; i < 3; i++)
        {
            cout << int(pt->endCHS[i]) << ',';
        }
        cout << dec << getLe32(pt->relativeSectors) << ',';
        cout << getLe32(pt->totalSectors) << endl;
    }
    if (!valid)
    {
        cout << '\t' << F("\nMBR not valid, assuming Super Floppy format.\n");
    }
    return true;
}

bool SdManager::mbr_check()
{
    MbrSector_t mbr;
    return sd.card()->readSector(0, (uint8_t *)&mbr);
}

bool SdManager::vol_dump()
{
    cout << '\t' << F("Scanning FAT, please wait.\n");
    uint32_t freeClusterCount = sd.freeClusterCount();
    if (sd.fatType() <= 32)
    {
        cout << '\t' << F("Volume is FAT") << int(sd.fatType()) << endl;
    }
    else
    {
        cout << '\t' << F("Volume is exFAT\n");
    }
    cout << '\t' << F("sectorsPerCluster: ") << sd.sectorsPerCluster() << endl;
    cout << '\t' << F("clusterCount:      ") << sd.clusterCount() << endl;
    cout << '\t' << F("freeClusterCount:  ") << freeClusterCount << endl;
    cout << '\t' << F("fatStartSector:    ") << sd.fatStartSector() << endl;
    cout << '\t' << F("dataStartSector:   ") << sd.dataStartSector() << endl;
    if (sd.dataStartSector() % m_eraseSize)
    {
        cout << F("Data area is not aligned on flash erase boundary!\n");
        cout << F("Download and use formatter from www.sdcard.org!\n");
    }
    return true;
}

void SdManager::error_print()
{
    if (sd.sdErrorCode())
    {
        cout << F("SD errorCode: ") << hex << showbase;
        printSdErrorSymbol(&Serial, sd.sdErrorCode());
        cout << F(" = ") << int(sd.sdErrorCode()) << endl;
        cout << F("SD errorData = ") << int(sd.sdErrorData()) << endl;
    }
}