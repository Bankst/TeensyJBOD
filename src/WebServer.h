#pragma once

#include <Arduino.h>
#include <NativeEthernet.h>
#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>
#include <SdFat.h>

#include <iostream>
#include <regex>
#include <iterator>

#include "SerialLogger.h"
#include "StringUtil.h"
#include "SensorUtils.h"

#define REQ_BUF_SZ 4096

class WebServer
{
private:
    EthernetServer server;
    SdFs *sd_fs;

public:
    WebServer() : server(EthernetServer(80)), sd_fs(nullptr){};
    WebServer(uint16_t port, SdFs *sd_fs) : server(EthernetServer(port)), sd_fs(sd_fs){};
    void init();
    void service();

private:
    bool parse_http_request(httpparser::Request &request, std::string request_raw_str);

    void service_requests();
    void format_response_body(std::string body, std::string regex_pattern, std::string content);
    void send_http_response(EthernetClient *client, int statusCode, std::string response);

    std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

    std::string default_index_response_body = R"HTML(
        <!DOCTYPE HTML>
        <html>
            <head>
                <title>TeensyJBOD</title>
            </head>
            <body>
                <h3>Nothing here yet...</h3>
            </body>
        </html>
    )HTML";

    std::string not_found_response_body = R"HTML(
        <!DOCTYPE HTML>
        <html>
            <head>
                <title>TeensyJBOD - 404</title>
            </head>
            <body>
                <h3>Nothing here!</h3>
            </body>
        </html>
    )HTML";
    

    std::string server_error_response_body = R"HTML(
        <!DOCTYPE HTML>
        <html>
            <head>
                <title>TeensyJBOD - 500</title>
            </head>
            <body>
                <h3>Server error</h3>
            </body>
        </html>
    )HTML";

    void log_debug(std::string str) { cout << F("[WebServer.h - DEBUG] ") << str.c_str() << endl; }
    void log_info(std::string str) { cout << F("[WebServer.h - INFO] ") << str.c_str() << endl; }
    void log_warn(std::string str) { cout << F("[WebServer.h - WARNING] ") << str.c_str() << endl; }
    void log_error(std::string str) { cout << F("[WebServer.h - ERROR] ") << str.c_str() << endl; }
};

std::string WebServer::ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}


void WebServer::init()
{
    server.begin();

    if (sd_fs != nullptr)
    {
        // load from SD
        log_info("Serving from SD");
    }
    else
    {
        // send 404/error page/smth
        log_info("Serving from flash - limited functionality will be available!");
    }
}

void WebServer::service()
{
    service_requests();
}

void WebServer::service_requests()
{
    EthernetClient client = server.available();
    if (client)
    {
        log_debug("client connected");

        if (client.connected())
        {
            bool request_complete = false;
            std::string request_raw_str = "";

            if (client.available())
            {
                while (!request_complete) {
                    int req_len = request_raw_str.length();

                    if (req_len >= 4 &&
                        request_raw_str.substr(req_len - 4, req_len) == "\r\n\r\n"
                    ) {
                        request_complete = true;
                    } else {
                        char c = client.read();
                        request_raw_str += c;
                    }
                }
            }

            // std::string debug_request = ReplaceAll(request_raw_str, "\r", "\\r");
            // debug_request = ReplaceAll(debug_request, "\n", "\\n\n");
            // cout << debug_request.c_str();

            // parse request
            httpparser::Request request;
            if (parse_http_request(request, request_raw_str))
            {
                log_info("Got request for \"" + request.uri + "\"");

                if (request.uri == "/" || request.uri.find("/index.htm") != std::string::npos) {
                    send_http_response(&client, 200, default_index_response_body);
                    log_info("URI found, sending 200");
                } else {
                    send_http_response(&client, 404, not_found_response_body);
                    log_info("URI not found, sending 404");
                }                
            } else {
                log_warn("bad HTTP request, sending 500");
                send_http_response(&client, 500, server_error_response_body);
            }

            // client.flush();
            delay(10);
            client.stop();
            log_debug("response sent - client disconnected");
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
}

bool WebServer::parse_http_request(httpparser::Request &request, std::string request_raw_str)
{
    httpparser::HttpRequestParser parser;

    const char * req_raw = request_raw_str.c_str();

    // cout << "raw HTTP req:\n" << req_raw;

    httpparser::HttpRequestParser::ParseResult res = parser.parse(request, req_raw, req_raw + strlen(req_raw));

    if (res == httpparser::HttpRequestParser::ParsingCompleted)
    {
        log_debug("parsed HTTP request");
        return true;
    }
    else if (res == httpparser::HttpRequestParser::ParsingIncompleted)
    {
        log_error("http parsing incomplete?!?!");
        return false;
    }
    else {
        log_error("failed to parse HTTP request");
        return false;
    }
}

void WebServer::format_response_body(std::string body, std::string regex_pattern, std::string content) {
    // TODO
    // return std::regex_replace();
}

void WebServer::send_http_response(EthernetClient *client, int statusCode, std::string response)
{
    switch (statusCode)
    {
    case 200:
    {
        client->println(F("HTTP/1.1 200 OK"));
        client->println(F("Content-Type: text/html"));
        client->println(F("Connection: keep-alive"));
        client->println();
        client->println(response.c_str());
        break;
    }
    case 404:
    {
        client->println(F("HTTP/1.1 404 Not Found"));
        client->println(F("Content-Type: text/html"));
        client->println(F("Connection: keep-alive"));
        client->println();
        client->println(response.c_str());
        break;
    }
    case 500:
    {
        client->println(F("HTTP/1.1 500 Internal Server Error"));
        client->println(F("Content-Type: text/html"));
        client->println(F("Connection: keep-alive"));
        client->println();
        client->println(response.c_str());
        break;
    }

    }
}