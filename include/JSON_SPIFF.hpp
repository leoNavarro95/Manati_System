
String readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return "ERROR";
    }
    String cadena;
    // Serial.println("- read from file:");
    while(file.available()){
        // Serial.write(file.read());
        cadena += (char) file.read();
    }
    // Serial.println(cadena);
    return cadena;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- frite failed");
    }
}

void appendFile(fs::FS &fs, const char * path, String message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
}

//Guarda la configuracion en la SPIFFS en config.json --Llamado desde WebSochets.hpp 
void guardarConfig(String SSID, String Pass, int Id){
    String paqueteJSON;
    const size_t capacity = JSON_OBJECT_SIZE(6) + 79;
    DynamicJsonDocument doc(capacity);

    doc["ssid"] = SSID;
    doc["id"] = Id;
    doc["pass"] = Pass;
    //Vuelve a tomar los valores ya configurados para no perderlos
    doc["nivelMinimo"] = NIVEL_BAJO;
    doc["nivelMaximo"] = NIVEL_ALTO;
    doc["flujoMinimo"] = UMBRAL_FLUJO;

    serializeJson(doc, paqueteJSON);//en paqueteJSON ya esta serializado el Json con los respectivos campos llenos

    // Serial.println("Guardando paquete de config: " + paqueteJSON);
    writeFile(SPIFFS, "/config.json", paqueteJSON.c_str());
    //luego se debe reiniciar el esp32 para que sean validos los cambios
    //quiero enviar un responce "OK" para decirle a la web que se admitió el cambio y si no un "ERROR"
    ESP.restart();


}

//Obtiene la configuracion desde el archivo config.json que se encuentra en la SPIFFS
void getConfig(){
    String DatoJSON = readFile(SPIFFS, "/config.json");
    if(DatoJSON == "ERROR"){
        Serial.print("Error leyendo archivo de config. Configuración tomada por defecto");
        
        String paqueteJSON;
        StaticJsonDocument<300> doc;
        doc["ssid"] = SSID;
        doc["id"] = 1;//1 indica que es por defecto, 2 que es custom
        doc["pass"] = PASSWORD;
        serializeJson(doc, paqueteJSON);//en paqueteJSON ya esta serializado el Json con los respectivos campos llenos

        // char paketeChar[paqueteJSON.length() + 1];
        // strcpy(paketeChar, paqueteJSON.c_str()); //convierte el packete en string a char array
        Serial.println("Guardando paquete de config: " + paqueteJSON);
        writeFile(SPIFFS, "/config.json", paqueteJSON.c_str());
    }
    else{
        StaticJsonDocument<300> doc;
        DeserializationError error = deserializeJson(doc, DatoJSON);
        if (error) { Serial.println("Error de parseado de Json"); return;}
        SSID = doc["ssid"] | "Sistema Manati ApError";//obtiene el valor de la etiqueta ssid, si no existe se settea una por defecto
        PASSWORD = doc["pass"] | "Administrador";
        int id = (int)doc["id"] | 1;
        if(id == 1)     Serial.println("Advertencia está usando configuración por defecto");

        NIVEL_ALTO = doc["nivelMaximo"] | 100;
        NIVEL_BAJO = doc["nivelMinimo"] | 70;
        UMBRAL_FLUJO = doc["flujoMinimo"] | 5;

        Serial.println("##1");
        Serial.println(String(NIVEL_ALTO) + " " + String(NIVEL_BAJO) + " " + String(UMBRAL_FLUJO));
        Serial.println("##1");
    }
    

}