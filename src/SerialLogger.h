#pragma once

#include <string>

namespace ASerialLogger
{
    class ASerialLogger
    {
    private:
        std::string context_name;

        std::string debug_prefix;
        std::string info_prefix;
        std::string warn_prefix;
        std::string error_prefix;

    public:
        ASerialLogger(std::string context_name) : context_name(context_name)
        {
            debug_prefix = "[" + context_name + " - DEBUG] ";
            info_prefix = "[" + context_name + " - INFO] ";
            warn_prefix = "[" + context_name + " - WARN] ";
            error_prefix = "[" + context_name + " - ERROR] ";
        };

        void debug(std::string str) { write(debug_prefix + str); }
        void info(std::string str) { write(info_prefix + str); }
        void warn(std::string str) { write(warn_prefix + str); }
        void error(std::string str) { write(error_prefix + str); }

    private:
        void write(std::string str)
        {
            // print here
        }
    };
};

using SerialLogger = ASerialLogger::ASerialLogger;
// ThreadWrap(Serial, SerialX);
// #define Serial ThreadClone(SerialX)

// ArduinoOutStream serialout(Serial);

// ThreadWrap(serialout, serialoutX);
// using serialout = ThreadClone(serialoutX);
// #define serialout ThreadClone(serialoutX)