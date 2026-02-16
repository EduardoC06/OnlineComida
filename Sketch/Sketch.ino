#include <NeoSWSerial.h>
#include <Servo.h>

// ================= DATOS DEL PROYECTO =================
const char* SSID_WIFI = "";        // Tu Wifi
const char* PASS_WIFI = "";               // Tu Clave
const char* IP_SERVER = "";  // Tu Laptop
const int   PUERTO    = 5000;

// Usamos NeoSWSerial en lugar de SoftwareSerial
// Pin 2 (RX del Arduino) recibe del TX del ESP
// Pin 3 (TX del Arduino) envía al RX del ESP
NeoSWSerial esp8266(2, 3); 
Servo miServo;

void setup() {
  Serial.begin(9600);    // Monitor Serie de PC
  
  // NeoSWSerial soporta 9600, 19200 y 38400 muy bien.
  // Asumimos que tu ESP ya está configurado a 9600 por los pasos anteriores.
  esp8266.begin(9600);   
  
  miServo.attach(9);
  miServo.write(0);
  delay(500);          // 3. Esperamos a que llegue
  miServo.detach();

  Serial.println("\n=== INICIANDO SISTEMA (NeoSWSerial) ===");

  // 1. Resetear para limpiar cualquier error previo
  enviarComando("AT+RST\r\n", 2000);
  enviarComando("AT+CWMODE=1\r\n", 1000);

  // 2. BUCLE DE CONEXIÓN WI-FI ROBUSTO
  bool conectado = false;
  while (!conectado) {
    Serial.print("📡 Conectando a WiFi: ");
    Serial.println(SSID_WIFI);
    
    // Limpiamos buffer antes de enviar
    while(esp8266.available()) esp8266.read();

    String cmd = "AT+CWJAP=\"" + String(SSID_WIFI) + "\",\"" + String(PASS_WIFI) + "\"\r\n";
    esp8266.print(cmd);
    
    // Esperamos hasta 8 segundos buscando "OK" o "GOT IP"
    long inicio = millis();
    bool exito = false;
    
    while(millis() - inicio < 8000) {
      if(esp8266.available()) {
        char c = esp8266.read();
        // Serial.write(c); // Descomenta si quieres ver todo el log
        if(c == 'K' || c == 'G') exito = true; // K de OK, G de GOT IP
      }
    }

    if(exito) {
      Serial.println("✅ CONEXIÓN WI-FI EXITOSA.");
      mostrarIP();
      conectado = true;
    } else {
      Serial.println("❌ FALLA. Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

void loop() {
  // 1. Conectar al Docker
  Serial.print(">> Conectando al Server... ");
  String cmd = "AT+CIPSTART=\"TCP\",\"" + String(IP_SERVER) + "\"," + String(PUERTO) + "\r\n";
  esp8266.print(cmd);
  delay(1000); 

  // 2. Enviar GET
  String peticion = "GET /api/arduino-check HTTP/1.1\r\nHost: " + String(IP_SERVER) + "\r\nConnection: close\r\n\r\n";
  String len = "AT+CIPSEND=" + String(peticion.length()) + "\r\n";
  
  esp8266.print(len);
  delay(500); // Esperar ">"
  
  // Limpiar buffer (NeoSWSerial es rápido, limpiamos basura vieja)
  while(esp8266.available()) esp8266.read();
  
  esp8266.print(peticion);
  
  // 3. Leer Respuesta
  long t = millis();
  bool abrir = false;
  
  // Escuchamos 2 segundos
  while((millis() - t) < 2000) {
    if(esp8266.available()) {
      char c = esp8266.read();
      // Serial.write(c); // Ver respuesta del server
      if(c == 'Y') abrir = true;
    }
  }

  // 4. Actuar
  if(abrir) {
    Serial.println("\n😺 ¡ALIMENTANDO!");
    miServo.attach(9);

    miServo.write(90);
    delay(2000);

    miServo.write(0);
    delay(1000);
    
    miServo.detach();
  } else {
    Serial.println("Nada.");
  }

  esp8266.print("AT+CIPCLOSE\r\n");
  delay(4000); // Esperar antes del siguiente ciclo
}

// --- AUXILIARES ---
void enviarComando(String cmd, int espera) {
  esp8266.print(cmd);
  delay(espera);
  while(esp8266.available()) esp8266.read();
}

void mostrarIP() {
  esp8266.print("AT+CIFSR\r\n");
  delay(1000);
  Serial.print("IP: ");
  while(esp8266.available()) {
    char c = esp8266.read();
    if(isdigit(c) || c == '.') Serial.write(c);
  }
  Serial.println();
}
