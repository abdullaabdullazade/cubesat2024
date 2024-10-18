#define BLYNK_TEMPLATE_ID "TMPL6BYql3Rn7"
#define BLYNK_TEMPLATE_NAME "Temperature and Humidity Monitor"
#define BLYNK_AUTH_TOKEN "Fnsutv56E2mgSljQAWRqkpZbYIzF5vt7"

#include <ArduinoWiFiServer.h>
//#include <AsyncTCP.h>  // ESP32 ilə birlikdə Asinxron TCP bağlantısı
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include  <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

#include <BlynkSimpleEsp8266.h>  // Updated for ESP8266

char auth[] = BLYNK_AUTH_TOKEN;

const char* ssid = "Kainat";  // WiFi adı
const char* password = "1N2S3A4N";  // WiFi şifrə
#define DHTPIN 5  // Sensor siqnal pini
#define DHTTYPE DHT11  // DHT sensor növü

BlynkTimer timer;
DHT dht(DHTPIN, DHTTYPE);

float t = 0.0;
float h = 0.0;

// AsyncWebServer obyektini 80 portunda yarat
AsyncWebServer server(80);
unsigned long previousMillis = 0;
const long interval = 10000;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
 <meta name="viewport" content="width=device-width, initial-scale=1">
 <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" crossorigin="anonymous">
 <style>
 html { font-family: Arial; text-align: center; }
 h2 { font-size: 3.0rem; }
 p { font-size: 3.0rem; }
 .units { font-size: 1.2rem; }
 .dht-labels { font-size: 1.5rem; vertical-align: middle; padding-bottom: 15px; }
 </style>
</head>
<body>
 <h2>OrionAZ</h2> 
 <p><i class="fas fa-thermometer-half" style="color:#059e8a;"></i> <span class="dht-labels">Temperature</span> <span id="temperature">%TEMPERATURE%</span><sup class="units">C</sup></p>
 <p><i class="fas fa-tint" style="color:#00add6;"></i> <span class="dht-labels">Humidity</span> <span id="humidity">%HUMIDITY%</span><sup class="units">%</sup></p>
</body>
<script>
setInterval(function () {
 var xhttp = new XMLHttpRequest();
 xhttp.onreadystatechange = function() {
   if (this.readyState == 4 && this.status == 200) {
     document.getElementById("temperature").innerHTML = this.responseText;
   }
 };
 xhttp.open("GET", "/temperature", true);
 xhttp.send();
}, 10000);
setInterval(function () {
 var xhttp = new XMLHttpRequest();
 xhttp.onreadystatechange = function() {
   if (this.readyState == 4 && this.status == 200) {
     document.getElementById("humidity").innerHTML = this.responseText;
   }
 };
 xhttp.open("GET", "/humidity", true);
 xhttp.send();
}, 10000);
</script>
</html>)rawliteral";

String processor(const String& var) {
  if (var == "TEMPERATURE") {
    return String(t);
  } else if (var == "HUMIDITY") {
    return String(h);
  }
  return String();
}

void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Məlumatları Blynk serverinə göndər
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  Serial.print("Temperature : ");
  Serial.print(t);
  Serial.print("    Humidity : ");
  Serial.println(h);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Wi-Fi bağlantısı
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println(WiFi.localIP());

  // Blynk-i işə sal
  Blynk.begin(auth, ssid, password);
  
  // Server kökləmələri
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", String(h).c_str());
  });

  // Serveri işə sal
  server.begin();
  
  // Sensor məlumatlarını göndərmək üçün taymer
  timer.setInterval(10000L, sendSensor);
}

void loop() {
  Blynk.run();
  timer.run();
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Temperatur və rütubət dəyərlərini yenilə
    float newT = dht.readTemperature();
    if (!isnan(newT)) {
      t = newT;
    }

    float newH = dht.readHumidity();
    if (!isnan(newH)) {
      h = newH;
    }
  }
}
