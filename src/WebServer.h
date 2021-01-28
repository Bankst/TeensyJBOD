#pragma once

#include "Arduino.h"
#include <NativeEthernet.h>
#include <SD.h>
#include <httpparser/request.h>


#define REQ_BUF_SZ 4096

class WebServer {
private:


    EthernetServer server;
    String sdWebRoot;
public:
    WebServer(uint16_t port, String sdWebRoot) : server(EthernetServer(port)), sdWebRoot(sdWebRoot) {};
    void init();
    void service();
private:

    char http_req_buf[REQ_BUF_SZ] = {0};
    int http_req_buf_index = 0;

    void service_requests();
    void send_http_response(EthernetClient * client, int statusCode, const char * response);

    const char * not_found_response_body = R"HTML(
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
};

void WebServer::init() {
    server.begin();

    // TODO: SD init
}

void WebServer::service() {
    service_requests();
}

void WebServer::service_requests() {
    EthernetClient client = server.available();
    if (client) {
        Serial.println("client connected");

        while(client.connected()) {
            if (client.available()) {

                if (http_req_buf_index < (REQ_BUF_SZ - 1)) {
                    http_req_buf[http_req_buf_index] = client.read();
                    http_req_buf_index++;
                } else {
                    Serial.println("Request too large for buffer!");
                }
            }

            // TODO: debug check, cleanup?
            // char lastChar;
            for (int i = 0; i < REQ_BUF_SZ; i++) {
                // end of request will look like { '\r', '\n', '0', '0', etc..}
                // if (http_req_buf[i] != '0' && lastChar != '\n') {
                    // lastChar = http_req_buf[i];
                    Serial.write(http_req_buf[i]);
                // }                
            }

            // TODO: parse HTTP request rather than just waiting for final line
            send_http_response(&client, 404, not_found_response_body);
        }
        delay(1);
        client.stop();
        Serial.println("client disconnected");
    }
}

void WebServer::send_http_response(EthernetClient * client, int statusCode, const char * response) {

    switch(statusCode) {
        case 200: {
            client->println("HTTP/1.1 200 OK");
            client->println("Content-Type: text/html");
            client->println("Connection: keep-alive");
            client->println();
            client->println(response);
            break;
        }
        case 404: {
            client->println("HTTP/1.1 404 Not Found");
            client->println("Content-Type: text/html");
            client->println("Connection: keep-alive");
            client->println();
            client->println(response);
            break;
        }
    }

}