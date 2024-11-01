#define BLYNK_TEMPLATE_ID "TMPL6BYql3Rn7"
#define BLYNK_TEMPLATE_NAME "Temperature and Humidity Monitor"
#define BLYNK_AUTH_TOKEN "Fnsutv56E2mgSljQAWRqkpZbYIzF5vt7"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>


char auth[] = BLYNK_AUTH_TOKEN;


const char* ssid = "Aqil";
const char* password = "11111111";


#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float t = 0.0, h = 0.0;


BlynkTimer timer;
Adafruit_MPU6050 mpu;


AsyncWebServer server(80);


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


void sendSensor() {
  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  
  Blynk.virtualWrite(V1, h);  
  Blynk.virtualWrite(V2, t);  

  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  
  Blynk.virtualWrite(V3, g.gyro.x);
  Blynk.virtualWrite(V4, g.gyro.y);
  Blynk.virtualWrite(V5, g.gyro.z);

  
  Blynk.virtualWrite(V6, a.acceleration.x);
  Blynk.virtualWrite(V7, a.acceleration.y);
  Blynk.virtualWrite(V8, a.acceleration.z);

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print("    Humidity: ");
  Serial.println(h);
  Serial.print("Gyro X: ");
  Serial.print(g.gyro.x);
  Serial.print(" Y: ");
  Serial.print(g.gyro.y);
  Serial.print(" Z: ");
  Serial.println(g.gyro.z);

  Serial.print("Accel X: ");
  Serial.print(a.acceleration.x);
  Serial.print(" Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(" Z: ");
  Serial.println(a.acceleration.z);
}

void setup() {
  Serial.begin(115200);
  dht.begin();


  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println(WiFi.localIP());


  Blynk.begin(auth, ssid, password);

  
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  timer.setInterval(10000L, sendSensor);

  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(t).c_str());
  });


  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(h).c_str());
  });


  server.begin();
}

void loop() {
  Blynk.run();
  timer.run();
}
