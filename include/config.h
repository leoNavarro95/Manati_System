
const char* SSID     = "Sistema Manati";
const char* PASSWORD = "Administrador";
// const char* hostname = "My_Esp32";

// variables configurables del tanque
uint8_t NIVEL_BAJO = 70;//nivel bajo porcentual a partir del cual enciende la bomba
uint8_t NIVEL_ALTO = 100;//nivel alto porcentual a partir del cual apaga la bomba
uint8_t UMBRAL_FLUJO = 5;//si el flujo medido es menor que este se dispara el error de bomba

IPAddress ip(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
