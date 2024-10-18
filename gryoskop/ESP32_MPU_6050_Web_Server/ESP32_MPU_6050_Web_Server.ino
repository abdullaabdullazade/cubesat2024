/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Tam layihə detalları üçün: https://RandomNerdTutorials.com/esp32-mpu-6050-web-server/
  Bu proqram təminatı və əlaqəli sənədlərin hər hansı bir kopyasını alan hər hansı bir şəxsə pulsuz olaraq verilən icazədir.
  Yuxarıdakı müəlliflik hüququ bildirimi və bu icazə bildirimi bütün nüsxələrdə və ya mühüm hissələrdə daxil edilməlidir.
*********/

#define BLYNK_TEMPLATE_ID "TMPL6A1B9Uopp"
#define BLYNK_TEMPLATE_NAME "Geleskop"
#define BLYNK_AUTH_TOKEN "UFadiDMmAydSKMFNt6OW5hQ_jP00rtxG"

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino_JSON.h>
#include "LittleFS.h"
#include <BlynkSimpleEsp32.h>

// Şəbəkə məlumatlarını dəyişin
const char *ssid = "Kainat";  // Şəbəkə adını daxil edin
const char *password = "1N2S3A4N";  // Şifrəni daxil edin

// Blynk autentifikasiyası
char auth[] = BLYNK_AUTH_TOKEN;

// Asinxron Veb Server obyekti yarat
AsyncWebServer server(80);

// /events ünvanında hadisə mənbəyi yaradın
AsyncEventSource events("/events");

// Sensor oxumaqları üçün JSON dəyişəni
JSONVar readings;

// Zaman dəyişənləri
unsigned long lastTime = 0;
unsigned long lastTimeAcc = 0;
unsigned long gyroDelay = 10;
unsigned long accelerometerDelay = 200;

// Sensor obyekti yaradın
Adafruit_MPU6050 mpu;

sensors_event_t a, g;

float gyroX = 0, gyroY = 0, gyroZ = 0;
float accX, accY, accZ;

// Gyroskop sensorunun dəyişikliyi
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;

// MPU6050-i başlat
void initMPU() {
  if (!mpu.begin()) {
    Serial.println("MPU6050 tapılmadı");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 tapıldı!");
}

void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS-i montaj edərkən xəta baş verdi");
  }
  Serial.println("LittleFS uğurla montaj edildi");
}

// Wi-Fi-ni başlat
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Wi-Fi-yə qoşulma...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
}

String getGyroReadings() {
  mpu.getEvent(&a, &g, nullptr);  // Akselerator məlumatları üçün nullptr verin

  float gyroX_temp = g.gyro.x;
  if (abs(gyroX_temp) > gyroXerror) {
    gyroX += gyroX_temp / 50.00;
  }

  float gyroY_temp = g.gyro.y;
  if (abs(gyroY_temp) > gyroYerror) {
    gyroY += gyroY_temp / 70.00;
  }

  float gyroZ_temp = g.gyro.z;
  if (abs(gyroZ_temp) > gyroZerror) {
    gyroZ += gyroZ_temp / 90.00;
  }

  readings["gyroX"] = String(gyroX);
  readings["gyroY"] = String(gyroY);
  readings["gyroZ"] = String(gyroZ);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

String getAccReadings() {
  mpu.getEvent(&a, &g, nullptr);  // Akselerator məlumatları üçün nullptr verin

  // Hal-hazırda olan sürət dəyərlərini əldə edin
  accX = a.acceleration.x;
  accY = a.acceleration.y;
  accZ = a.acceleration.z;
  readings["accX"] = String(accX);
  readings["accY"] = String(accY);
  readings["accZ"] = String(accZ);

  String accString = JSON.stringify(readings);
  return accString;
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  Blynk.begin(auth, ssid, password);  // Blynk-i başlat
  initLittleFS();
  initMPU();

  // Veb Serveri idarə edin
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroX = 0;
    gyroY = 0;
    gyroZ = 0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroX = 0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroY = 0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroZ = 0;
    request->send(200, "text/plain", "OK");
  });

  // Veb Server Hadisələrini idarə edin
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      Serial.printf("Müştəri yenidən qoşuldu! Son mesaj ID: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  server.begin();
}

void loop() {
  Blynk.run();  // Blynk-i işə salın

  if ((millis() - lastTime) > gyroDelay) {
    // Sensor oxumaqları ilə Veb Serverə hadisələr göndərin
    events.send(getGyroReadings().c_str(), "gyro_readings", millis());

    // Blynk məlumatlarını göndərin
    Blynk.virtualWrite(V0, gyroX);  // gyroX-i v0-a göndərin
    Blynk.virtualWrite(V1, gyroY);  // gyroY-i v1-a göndərin
    Blynk.virtualWrite(V2, gyroZ);  // gyroZ-i v2-yə göndərin
    lastTime = millis();
  }
  if ((millis() - lastTimeAcc) > accelerometerDelay) {
    // Sensor oxumaqları ilə Veb Serverə hadisələr göndərin
    events.send(getAccReadings().c_str(), "accelerometer_readings", millis());

    // Blynk məlumatlarını göndərin
    Blynk.virtualWrite(V3, accX);  // accX-i v3-ə göndərin
    Blynk.virtualWrite(V4, accY);  // accY-i v4-ə göndərin
    lastTimeAcc = millis();
  }
}
