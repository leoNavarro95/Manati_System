AsyncWebServer server(80);

void InitServer()
{
// 	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){// Route handling function
// 	AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
// 	response->addHeader("Content-Encoding", "gzip"); 
// 	request->send(SPIFFS, "/bundle.gz", String(), false);
// });
server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(SPIFFS, "/bundle.gz", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });
	

	server.serveStatic("/", SPIFFS, "/").setDefaultFile("bundle.gz");

	server.onNotFound([](AsyncWebServerRequest *request) {
		request->send(400, "text/plain", "Not found");
	});

	server.begin();
    Serial.println("HTTP server started");
}
