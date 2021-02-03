#pragma once

#include <Arduino.h>
#include <NativeEthernet.h>
#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>
#include <SdFat.h>

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
    void send_http_response(EthernetClient *client, int statusCode, std::string response);

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

        digitalWrite(LED_BUILTIN, HIGH);

        if (client.connected())
        {
            bool request_complete = false;
            std::string request_raw_str = "";

            if (client.available())
            {
                digitalWrite(LED_BUILTIN, LOW);
                // bool cur_line_blank = false;
                // while (!request_complete) {
                //     char c = client.read();


                //     int req_len = request_raw_str.length();
                //     if (request_raw_str.at(req_len - 3) == '\r' && 
                //         request_raw_str.at(req_len - 2) == '\n' && 
                //         request_raw_str.at(req_len - 1) == '\r' && 
                //         request_raw_str.at(req_len) == '\n'
                //     ) {
                //         request_complete = true;
                //     } else {
                //         request_raw_str += c;
                //     }


                    
                //     // if (c == '\n' && cur_line_blank) {
                //     //     request_complete = true;
                //     // } else if (c == '\n') {
                //     //     cur_line_blank = true;
                //     // } else if (c != '\r') {
                //     //     cur_line_blank = false;
                //     // }
                // }
            }

            // parse request
            httpparser::Request request;
            if (parse_http_request(request, request_raw_str))
            {
                // log_debug("HTTP Request:\n" + request.inspect());

                // TODO: parse URI

                send_http_response(&client, 404, not_found_response_body);
            } else {
                log_warn("bad HTTP request, sending 500");
                send_http_response(&client, 500, server_error_response_body);
            }

            // client.flush();
            delay(1);
            client.stop();
            log_debug("response sent - client disconnected");
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
}

bool WebServer::parse_http_request(httpparser::Request &request, std::string request_raw_str)
{
    cout << "raw HTTP req:\n" << request_raw_str.c_str() << endl;

    httpparser::HttpRequestParser parser;
    httpparser::HttpRequestParser::ParseResult res = parser.parse(request, request_raw_str.c_str(), request_raw_str.c_str() + sizeof(request_raw_str));

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