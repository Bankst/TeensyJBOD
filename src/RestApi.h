#pragma once

#include "WebServer.h"

#include <string>

class RestApi
{
private:
    WebServer *webServer;

public:
    RestApi(WebServer *webServer) : webServer(webServer)
    {
        webServer->register_endpoint("GET", "/api/version", this, handle_get_version_cb);
        webServer->register_endpoint("GET", "/api/hardware", this, handle_get_hwinfo_cb);
    }

private:
    int handle_get_version(httpparser::Request *request, std::string *response);
    static int handle_get_version_cb(void *context, httpparser::Request *request, std::string *response)
    {
        return reinterpret_cast<RestApi *>(context)->handle_get_version(request, response);
    }

    int handle_get_hwinfo(httpparser::Request *request, std::string *response);
    static int handle_get_hwinfo_cb(void *context, httpparser::Request *request, std::string *response)
    {
        return reinterpret_cast<RestApi *>(context)->handle_get_hwinfo(request, response);
    }
};

int RestApi::handle_get_version(httpparser::Request *request, std::string *response)
{
    response->append("{\"version\": \"0.1.200\"}");
    return 200;
}

int RestApi::handle_get_hwinfo(httpparser::Request *request, std::string *response)
{
    response->append(
        "{"
        "\"cpu_temp\": \"" + StringUtil::to_string(get_cpu_temp()) + " °C\","
        "}"
    );
    
    return 200;
}