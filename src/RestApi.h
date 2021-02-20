#pragma once

#include "WebServer.h"
#include "SdManager.h"

#include <string>

class RestApi
{
private:
    WebServer *webServer;
    SdManager *sdManager;

public:
    RestApi(WebServer *webServer, SdManager *sdManager) : webServer(webServer), sdManager(sdManager)
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
        "\"cpu_temp\": \"" + to_string(get_cpu_temp()) + " °C\","
        "\"sd_status\": \"" + to_string(sdManager->get_sd_ok()) + "\""
        "}"
    );
    
    return 200;
}