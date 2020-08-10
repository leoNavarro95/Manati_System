AsyncWebSocket ws("/ws");

//envia las variables de estado del tanque, estado de la bomba, de los sensores, umbrales, errores, etc...
void sendStateTank(){
  String response;
  const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(2) + 6*JSON_OBJECT_SIZE(3) + 365;
  DynamicJsonDocument doc(capacity);

  doc["command"] = "stateTank"; // OJO es lo que se revisa en el frontend

  JsonArray estado = doc.createNestedArray("estado");

  JsonObject estado_0 = estado.createNestedObject();
  estado_0["label"] = "Bomba";
  estado_0["varName"] = "pumpState";
  estado_0["value"] = pump_on ? String("Encendida") : String("Apagada");

  JsonObject estado_1 = estado.createNestedObject();
  estado_1["label"] = "Sensor de nivel";
  estado_1["varName"] = "errorLevelSens";
  estado_1["value"] = errorUltrasonico ? String("Error") : String("OK");

  JsonObject estado_2 = estado.createNestedObject();
  estado_2["label"] = "Sensor de flujo";
  estado_2["varName"] = "errorFlowSens";
  estado_2["value"] = errorFlujometro ? String("Error") : String("OK");

  JsonObject estado_3 = estado.createNestedObject();
  estado_3["label"] = "Umbral nivel bajo";
  estado_3["varName"] = "lowLevelThreshold";
  estado_3["value"] = (uint8_t) NIVEL_BAJO;

  JsonObject estado_4 = estado.createNestedObject();
  estado_4["label"] = "Umbral nivel alto";
  estado_4["varName"] = "highLevelThreshold";
  estado_4["value"] = (uint8_t) NIVEL_ALTO;

  JsonObject estado_5 = estado.createNestedObject();
  estado_5["label"] = "Umbral Flujo Bajo";
  estado_5["varName"] = "lowFlowThreshold";
  estado_5["value"] = (uint8_t) UMBRAL_FLUJO;

  serializeJson(doc, response);
  ws.textAll(response);// esto envia lo contenido en responce a todos los clientes via WebSockets
}


//esta funcion maneja todas las peticiones de la web que vienen codificadas en JSON
//segun el valor de command ejecuta una u otra funcion
//OJO la mayoria de las acciones a ejecutar van a estar definidas en la API.h
void ProcessRequest(AsyncWebSocketClient *client, String request)
{
  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, request);
  if (error) {Serial.print(F("deserializeJson() failed with code "));Serial.println(error.c_str()); return; }
  
  String command = doc["command"] | "NULL";//obtiene el valor de la etiqueta command, si no existe retorna un NULL
  if(command == "NULL")
    Serial.println("Error No existe comando en paquete json");
  else if(command == "setGPIO") 
    setGPIO(doc["id"], (bool)doc["status"]);
  else if(command == "setPWM")
    setPWM(doc["id"], (int)doc["pwm"]);
  else if(command == "doAction")
    doAction(doc["id"]);
  else if(command == "guardarConfig") //en JSON_SPIFF.hpp
    guardarConfig(doc["ssid"]|"Sistema Manati E",doc["pass"]|"Administrador",(int)doc["id"] | 1);//funcion declarada en JSON_SPIFF.hpp
  else if(command == "getStateTank") //pedido de obtener el estado del sistema
    sendStateTank();
  else if(command == "actualizarUmbrales")
    guardarUmbrales((uint8_t)doc["nivelMin"],(uint8_t)doc["nivelMax"],(uint8_t)doc["flujoMin"]); //en API.hpp
}

//este metodo envia un paquete serializado JSON a todos los clientes
void updateGPIO(String input, bool value)
{
  String response;
  //se crea el paquete JSON con la estructura:
  /*{ command: "updateGPIO",
      id: input, esto seria el string que se le pasaa como primer parametro a esta funcion
      status: en dependencia de value, si ese es "Encendido" retorna true, y si es apagado false
    }*/
  StaticJsonDocument<300> doc;
  doc["command"] = "updateGPIO";
  doc["id"] = input;
  doc["status"] = value ? String("Encendido") : String("Apagado");
  //se serializa el JSON contenido en doc y se almacena en el String responce
  serializeJson(doc, response);

  ws.textAll(response);// esto envia lo contenido en responce a todos los clientes via WebSockets

  Serial.print(input);
  Serial.println(value ? String(" Encendido") : String(" Apagado"));
}

//va a enviar a todos los clientes los valores leidos por el sensor
//el id va a servir para identificar a la temperatura(1), a la humedad(2) y al indice de Temp(3)
void updateSensorValue(String id, String value){
  String response;
  StaticJsonDocument<300> doc;
  doc["command"] = "updateSensor";
  doc["id"] = id;
  doc["value"] = value;
  //se serializa el JSON contenido en doc y se almacena en el String responce
  serializeJson(doc, response);

  ws.textAll(response);// esto envia lo contenido en responce a todos los clientes via WebSockets

  Serial.println("Sensor" + id + ": " + value);

}

void actualizaNivel(uint8_t nivel){
  String response;
  StaticJsonDocument<300> doc;
  doc["command"] = "actualizaNivel";
  doc["id"] = 4;
  doc["value"] = nivel;
  // doc["action"] = "countIncrement";
  //se serializa el JSON contenido en doc y se almacena en el String responce
  serializeJson(doc, response);
  ws.textAll(response);// esto envia lo contenido en responce a todos los clientes via WebSockets
}
