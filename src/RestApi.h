#pragma once

#include "WebServer.h"

class RestApi
{
private:

public:
    RestApi(WebServer * webServer);
private:
    int handle_get_version(httpparser::Request * request, std::string * response);
};

RestApi::RestApi(WebServer * webServer) 
{
    auto handle_get_version_cb = [this](httpparser::Request *request, std::string* response){return handle_get_version(request, response);};
    webServer->register_endpoint("GET", "/version", handle_get_version_cb);
}