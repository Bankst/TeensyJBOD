#pragma once

// #include <Arduino.h>
#include <NativeEthernet.h>
#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>
#include <SdFat.h>

#include <fnet_stdlib.h>
#include <iostream>
#include <regex>
#include <iterator>
#include <map>
// #include <unordered_map>

#include "SerialLogger.h"
#include "StringUtil.h"
#include "SensorUtils.h"

/* #region HTML bodies */
static std::string default_index_response_body = R"HTML(
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

static std::string not_found_response_body = R"HTML(
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

static std::string server_error_response_body = R"HTML(
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
/* #endregion */

using http_callback = std::function<int(httpparser::Request *, std::string *)>;
using http_request_info = std::pair<std::string, std::string>;

class WebServer
{
private:
    EthernetServer server;

    std::map<http_request_info, http_callback> registered_callbacks;

public:
    WebServer() : server(EthernetServer(80)){};
    WebServer(uint16_t port) : server(EthernetServer(port)){};
    void init();
    void service();

    void register_endpoint(const std::string &method, const std::string &uri, http_callback callback);
private:
    void service_requests();

    bool parse_http_request(httpparser::Request &request, std::string request_raw_str);
    void send_http_response(EthernetClient *client, int statusCode, std::string response, bool is_json = false);
    int handle_http_request(httpparser::Request * request, std::string * response);

    void log_debug(std::string str) { cout << F("[WebServer.h - DEBUG] ") << str.c_str() << endl; }
    void log_info(std::string str) { cout << F("[WebServer.h - INFO] ") << str.c_str() << endl; }
    void log_warn(std::string str) { cout << F("[WebServer.h - WARNING] ") << str.c_str() << endl; }
    void log_error(std::string str) { cout << F("[WebServer.h - ERROR] ") << str.c_str() << endl; }
};

void WebServer::init()
{
    server.begin();
    log_info("Web service init OK");
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
            std::string request_raw_str = "";

            int avail = client.available();

            if (avail)
            {
                request_raw_str = client.readString(avail).c_str();
            }

            std::string debug_request = ReplaceAll(request_raw_str, "\r", "\\r");
            debug_request = ReplaceAll(debug_request, "\n", "\\n\n");
            cout << "Raw HTTP Request:" << endl;
            cout << debug_request.c_str();

            // parse request
            httpparser::Request request;
            if (parse_http_request(request, request_raw_str))
            {
                log_info("Got request for \"" + request.uri + "\"");

                // TODO: find files
                if (request.uri == "/" || request.uri.find("/index.htm") != std::string::npos)
                {
                    send_http_response(&client, 200, default_index_response_body);
                    log_info("URI found, sending 200");
                }
                else
                {
                    send_http_response(&client, 404, not_found_response_body);
                    log_info("URI not found, sending 404");
                }
            }
            else
            {
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

    const char *req_raw = request_raw_str.c_str();

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
    else
    {
        log_error("failed to parse HTTP request");
        return false;
    }
}

void WebServer::send_http_response(EthernetClient *client, int statusCode, std::string response, bool is_json)
{
    std::string content_type = "Content-Type: ";
    content_type += (is_json ? "application/json" : "text/html");

    switch (statusCode)
    {
    case 200:
    {
        client->println(F("HTTP/1.1 200 OK"));
        client->println(content_type.c_str());
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

void WebServer::register_endpoint(const std::string &method, const std::string &uri, http_callback callback)
{
    auto req_info = std::make_pair(method, uri);
    registered_callbacks[req_info] = callback;
}

int WebServer::handle_http_request(httpparser::Request * request, std::string * response)
{
    auto req_info = std::make_pair(request->method, request->uri);
    auto it = registered_callbacks.find(req_info);

    if (it == registered_callbacks.end()) {
        return 404;
    }

    return it->second(request, response);
}