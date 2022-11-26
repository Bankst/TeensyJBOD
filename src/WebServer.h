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

using http_callback = int (*)(void *, httpparser::Request *, std::string *);
using http_request_info = std::pair<std::string, std::string>;
using http_callback_info = std::pair<void *, http_callback>;

class WebServer
{
private:
    EthernetServer server;

    std::map<http_request_info, http_callback_info> registered_callbacks;

public:
    WebServer() : WebServer(80){};
    WebServer(uint16_t port) : server(EthernetServer(port)){};
    void init();
    void service();

    void register_endpoint(const std::string &method, const std::string &uri, void *context, http_callback callback);

private:
    void service_requests();

    bool parse_http_request(httpparser::Request &request, std::string request_raw_str);
    void send_http_response(EthernetClient *client, int statusCode, std::string response, bool is_json = false);
    int handle_http_request(httpparser::Request *request, std::string *response);

    void log_debug(std::string str){};
    void log_info(std::string str){};
    void log_warn(std::string str){};
    void log_error(std::string str){};

    // void log_debug(std::string str) { serialout << F("[WebServer.h - DEBUG] ") << str.c_str() << endl; }
    // void log_info(std::string str) { serialout << F("[WebServer.h - INFO] ") << str.c_str() << endl; }
    // void log_warn(std::string str) { serialout << F("[WebServer.h - WARNING] ") << str.c_str() << endl; }
    // void log_error(std::string str) { serialout << F("[WebServer.h - ERROR] ") << str.c_str() << endl; }
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
    if (client.connected())
    {
#ifdef WEB_DEBUG
        log_debug("client connected");
        uint32_t client_begin_micros = micros();
#endif

        std::string request_raw_str = "";

        int avail = client.available();
        if (avail)
        {
            request_raw_str = client.readString(avail).c_str();
        }

#ifdef WEB_DEBUG
        std::string debug_request = ReplaceAll(request_raw_str, "\r", "\\r");
        debug_request = ReplaceAll(debug_request, "\n", "\\n\n");
        cout << "Raw HTTP Request:" << endl;
        cout << debug_request.c_str();
#endif

        // parse request
        httpparser::Request request;
        if (parse_http_request(request, request_raw_str))
        {
            log_info("Got " + request.method + " request for \"" + request.uri + "\"");

            if (request.uri == "/" || request.uri.find("/index.htm") != std::string::npos)
            {
                log_info("HTTP URI found, sending 200");
                send_http_response(&client, 200, default_index_response_body);
            }
            else
            {
                // search for URI in registered_callbacks
                http_request_info req_info = std::make_pair(request.method, request.uri);
                bool has_handler = registered_callbacks.count(req_info);

                if (has_handler)
                {
                    std::string response = "";
                    int resp_code = handle_http_request(&request, &response);
                    log_info("REST URI found, sending 200");
                    send_http_response(&client, resp_code, response, true);
                }
                else
                {
                    log_info("URI not found, sending 404");
                    send_http_response(&client, 404, not_found_response_body);
                }
            }
        }
        else
        {
            log_warn("bad HTTP request, sending 500");
            send_http_response(&client, 500, server_error_response_body);
        }

        client.stop();
#ifdef WEB_DEBUG
        uint32_t t = micros() - client_begin_micros;
        log_debug("response sent, client disconnected: " + to_string(t) + " us");
        threads.delay(1);
#endif
    }
}

bool WebServer::parse_http_request(httpparser::Request &request, std::string request_raw_str)
{
    httpparser::HttpRequestParser parser;

    const char *req_raw = request_raw_str.c_str();

    httpparser::HttpRequestParser::ParseResult res = parser.parse(request, req_raw, req_raw + strlen(req_raw));

    if (res == httpparser::HttpRequestParser::ParsingCompleted)
    {
#ifdef WEB_DEBUG
        log_debug("parsed HTTP request");
#endif
        return true;
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
        client->println(F("Connection: keep-alive\n"));
        client->println(response.c_str());
        break;
    }
    case 404:
    {
        client->println(F("HTTP/1.1 404 Not Found"));
        client->println(F("Content-Type: text/html"));
        client->println(F("Connection: keep-alive\n"));
        client->println(response.c_str());
        break;
    }
    case 500:
    {
        client->println(F("HTTP/1.1 500 Internal Server Error"));
        client->println(F("Content-Type: text/html"));
        client->println(F("Connection: keep-alive\n"));
        client->println(response.c_str());
        break;
    }
    }
}

void WebServer::register_endpoint(const std::string &method, const std::string &uri, void *context, http_callback callback)
{
    auto req_info = std::make_pair(method, uri);
    auto callback_info = std::make_pair(context, callback);
    registered_callbacks[req_info] = callback_info;
}

int WebServer::handle_http_request(httpparser::Request *request, std::string *response)
{
    auto req_info = std::make_pair(request->method, request->uri);
    auto it = registered_callbacks.find(req_info);

    if (it == registered_callbacks.end())
    {
        return 404;
    }

    auto context = it->second.first;
    auto callback = it->second.second;

    return callback(context, request, response);
}