// #pragma once

// #include <Arduino.h>
// #include <SdFat.h>
// #include "SerialLogger.h"

// #define SD_FAT_TYPE 0
// #define SD_CONFIG SdioConfig(FIFO_SDIO)
// const uint8_t SD_CS_PIN = SS;

// class SdManager
// {
// private:
//     SdFat sd;
//     // SD info
//     cid_t m_cid;
//     csd_t m_csd;
//     uint32_t m_eraseSize;
//     uint32_t m_ocr;

// public:
//     SdManager(){};
//     bool begin();
//     void print_info(bool detail);

//     bool get_sd_ok() { return sd_ok; }

//     SdFat *get_fs();

// private:
//     bool sd_ok;

//     std::string get_card_type();
//     void print_card_type();

//     void cid_dump();
//     bool csd_dump();
//     bool mbr_dump();
//     bool vol_dump();

//     bool mbr_check();

//     void error_print();

//     void log_debug(std::string str) { serialout << F("[SdManager.h - DEBUG] ") << str.c_str() << endl; }
//     void log_info(std::string str) { serialout << F("[SdManager.h - INFO] ") << str.c_str() << endl; }
//     void log_warn(std::string str) { serialout << F("[SdManager.h - WARNING] ") << str.c_str() << endl; }
//     void log_error(std::string str) { serialout << F("[SdManager.h - ERROR] ") << str.c_str() << endl; }
// };

// bool SdManager::begin()
// {
//     sd_ok = false;
//     uint32_t init_begin_millis = millis();
//     if (!sd.cardBegin(SD_CONFIG))
//     {
//         log_error("SD init failed!");
//         return false;
//     }

//     if (!sd.card()->readCID(&m_cid) ||
//         !sd.card()->readCSD(&m_csd) ||
//         !sd.card()->readOCR(&m_ocr))
//     {
//         log_error("readInfo failed");
//         error_print();
//         return false;
//     }

//     if (!mbr_check())
//     {
//         log_error("MBR init failed");
//         return false;
//     }

//     if (!sd.volumeBegin())
//     {
//         log_error("volumeBegin failed. Is the card formatted?");
//         error_print();
//         return false;
//     }

//     log_info("SD init OK: " + to_string(millis() - init_begin_millis) + " ms");
//     sd_ok = true;
//     return true;
// }

// void SdManager::print_info(bool detail)
// {
//     serialout << F("[SdManager.h]: Card info - ");

//     if (detail)
//     {
//         serialout << endl;
//         print_card_type();
//         cid_dump();
//         if (csd_dump())
//         {
//             if (mbr_dump())
//             {
//                 vol_dump();
//             }
//         }
//     }
//     else
//     {
//         double capacity = 0.000512 * sdCardCapacity(&m_csd);
//         serialout << capacity << " MB"
//                   << " " << get_card_type().c_str() << endl;
//     }
// }

// SdFat *SdManager::get_fs()
// {
//     return &sd;
// }

// std::string SdManager::get_card_type()
// {
//     switch (sd.card()->type())
//     {
//     case SD_CARD_TYPE_SD1:
//         return "SD1";
//     case SD_CARD_TYPE_SD2:
//         return "SD2";
//     case SD_CARD_TYPE_SDHC:
//         return (sdCardCapacity(&m_csd) < 70000000) ? "SDHC" : "SDXC";
//     default:
//         return "Unknown";
//     }
// }

// void SdManager::print_card_type()
// {
//     serialout << F("\tCard type: ");

//     switch (sd.card()->type())
//     {
//     case SD_CARD_TYPE_SD1:
//         serialout << F("SD1\n");
//         break;

//     case SD_CARD_TYPE_SD2:
//         serialout << F("SD2\n");
//         break;

//     case SD_CARD_TYPE_SDHC:
//         if (sdCardCapacity(&m_csd) < 70000000)
//         {
//             serialout << F("SDHC\n");
//         }
//         else
//         {
//             serialout << F("SDXC\n");
//         }
//         break;

//     default:
//         serialout << F("Unknown\n");
//     }
// }

// void SdManager::cid_dump()
// {
//     serialout << '\t' << F("Manufacturer ID: ");
//     serialout << uppercase << showbase << hex << int(m_cid.mid) << dec << endl;
//     serialout << '\t' << F("OEM ID: ") << m_cid.oid[0] << m_cid.oid[1] << endl;
//     serialout << '\t' << F("Product: ");
//     for (uint8_t i = 0; i < 5; i++)
//     {
//         serialout << m_cid.pnm[i];
//     }
//     serialout << endl;
//     serialout << '\t' << F("Version: ");
//     serialout << int(m_cid.prv_n) << '.' << int(m_cid.prv_m) << endl;
//     serialout << '\t' << F("Serial number: ") << hex << m_cid.psn << dec << endl;
//     serialout << '\t' << F("Manufacturing date: ");
//     serialout << int(m_cid.mdt_month) << '/';
//     serialout << (2000 + m_cid.mdt_year_low + 10 * m_cid.mdt_year_high) << endl;
// }

// bool SdManager::csd_dump()
// {
//     bool eraseSingleBlock;
//     if (m_csd.v1.csd_ver == 0)
//     {
//         eraseSingleBlock = m_csd.v1.erase_blk_en;
//         m_eraseSize = (m_csd.v1.sector_size_high << 1) | m_csd.v1.sector_size_low;
//     }
//     else if (m_csd.v2.csd_ver == 1)
//     {
//         eraseSingleBlock = m_csd.v2.erase_blk_en;
//         m_eraseSize = (m_csd.v2.sector_size_high << 1) | m_csd.v2.sector_size_low;
//     }
//     else
//     {
//         serialout << F("m_csd version error\n");
//         return false;
//     }
//     m_eraseSize++;
//     serialout << '\t' << F("Capacity: ") << 0.000512 * sdCardCapacity(&m_csd);
//     serialout << F(" MB (MB = 1,000,000 bytes)\n");

//     serialout << '\t' << F("flashEraseSize: ") << int(m_eraseSize) << F(" blocks\n");
//     serialout << '\t' << F("eraseSingleBlock: ");
//     if (eraseSingleBlock)
//     {
//         serialout << F("true\n");
//     }
//     else
//     {
//         serialout << F("false\n");
//     }
//     return true;
// }

// bool SdManager::mbr_dump()
// {
//     MbrSector_t mbr;
//     bool valid = true;
//     if (!sd.card()->readSector(0, (uint8_t *)&mbr))
//     {
//         serialout << F("\nread MBR failed.\n");
//         error_print();
//         return false;
//     }
//     serialout << F("\n\tSD Partition Table\n");
//     serialout << F("\tpart,boot,bgnCHS[3],type,endCHS[3],start,length\n");
//     for (uint8_t ip = 1; ip < 5; ip++)
//     {
//         MbrPart_t *pt = &mbr.part[ip - 1];
//         if ((pt->boot != 0 && pt->boot != 0X80) ||
//             getLe32(pt->relativeSectors) > sdCardCapacity(&m_csd))
//         {
//             valid = false;
//         }
//         serialout << '\t' << int(ip) << ',' << uppercase << showbase << hex;
//         serialout << int(pt->boot) << ',';
//         for (int i = 0; i < 3; i++)
//         {
//             serialout << int(pt->beginCHS[i]) << ',';
//         }
//         serialout << int(pt->type) << ',';
//         for (int i = 0; i < 3; i++)
//         {
//             serialout << int(pt->endCHS[i]) << ',';
//         }
//         serialout << dec << getLe32(pt->relativeSectors) << ',';
//         serialout << getLe32(pt->totalSectors) << endl;
//     }
//     if (!valid)
//     {
//         serialout << '\t' << F("\nMBR not valid, assuming Super Floppy format.\n");
//     }
//     return true;
// }

// bool SdManager::mbr_check()
// {
//     MbrSector_t mbr;
//     return sd.card()->readSector(0, (uint8_t *)&mbr);
// }

// bool SdManager::vol_dump()
// {
//     serialout << '\t' << F("Scanning FAT, please wait.\n");
//     uint32_t freeClusterCount = sd.freeClusterCount();
//     if (sd.fatType() <= 32)
//     {
//         serialout << '\t' << F("Volume is FAT") << int(sd.fatType()) << endl;
//     }
//     else
//     {
//         serialout << '\t' << F("Volume is exFAT\n");
//     }
//     serialout << '\t' << F("sectorsPerCluster: ") << sd.sectorsPerCluster() << endl;
//     serialout << '\t' << F("clusterCount:      ") << sd.clusterCount() << endl;
//     serialout << '\t' << F("freeClusterCount:  ") << freeClusterCount << endl;
//     serialout << '\t' << F("fatStartSector:    ") << sd.fatStartSector() << endl;
//     serialout << '\t' << F("dataStartSector:   ") << sd.dataStartSector() << endl;
//     if (sd.dataStartSector() % m_eraseSize)
//     {
//         serialout << F("Data area is not aligned on flash erase boundary!\n");
//         serialout << F("Download and use formatter from www.sdcard.org!\n");
//     }
//     return true;
// }

// void SdManager::error_print()
// {
//     if (sd.sdErrorCode())
//     {
//         serialout << F("SD errorCode: ") << hex << showbase;
//         printSdErrorSymbol(&Serial, sd.sdErrorCode());
//         serialout << F(" = ") << int(sd.sdErrorCode()) << endl;
//         serialout << F("SD errorData = ") << int(sd.sdErrorData()) << endl;
//     }
// }