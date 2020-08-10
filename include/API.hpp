
void setGPIO(String id, bool state)
{
    Serial.print("Set GPIO ");
    Serial.print(id);
    Serial.print(": ");
    Serial.println(state);

#ifdef OLED_DEBUG
    display.clear();
    vTaskDelay(pdMS_TO_TICKS(100));
    if (state)
    {
        display.drawString(0, 10, "LED Encendido");
    }
    else
    {
        display.drawString(0, 10, "LED Apagado");
    }
    display.display();

#endif
}

void setPWM(String id, int pwm)
{

    Serial.print("Set PWM ");
    Serial.print(id);
    Serial.print(": ");
    Serial.println(pwm);
#ifdef OLED_DEBUG
    display.clear();
    display.drawString(0, 20, "PWM: " + String(pwm));
    display.display();

#endif
}

void doAction(String actionId)
{
    Serial.print("Doing action: ");
    Serial.println(actionId);
}

void guardarUmbrales(uint8_t nivelmin, uint8_t nivelmax, uint8_t flujomin)
{

    String paqueteJSON;
    const size_t capacity = JSON_OBJECT_SIZE(6) + 79;
    DynamicJsonDocument doc(capacity);

    doc["nivelMinimo"] = nivelmin;
    doc["nivelMaximo"] = nivelmax;
    doc["flujoMinimo"] = flujomin;

    //Vuelve a tomar los valores ya configurados para no perderlos
    doc["ssid"] = mySsid.c_str();
    doc["id"] = 2;
    doc["pass"] = myPass.c_str();

    serializeJson(doc, paqueteJSON); //en paqueteJSON ya esta serializado el Json con los respectivos campos llenos

    // Serial.println("Guardando paquete de config: " + paqueteJSON);
    writeFile(SPIFFS, "/config.json", paqueteJSON.c_str());

    vTaskDelay(pdMS_TO_TICKS(500));
    getConfig(); //obtiene la configuracion desde la SPIFFS
    
}