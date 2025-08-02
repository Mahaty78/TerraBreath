#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi Credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// Pins
#define ONE_WIRE_BUS D2
const int soilMoisturePin = A0;
const int tdsSensorPin = A0;  // For ESP8266, only one analog pin

// OneWire and Temp Sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

// Web server on port 80
ESP8266WebServer server(80);

// Calibration
const int moistureDry = 1023;
const int moistureWet = 350;
const float VREF = 3.3;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  tempSensor.begin();

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  // Define server route
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  float temp = readTemperature();
  float moisture = readSoilMoisture();
  float tds = readTDS(analogRead(soilMoisturePin));  // Reusing analog pin

  String html = "<html><head><title>TerraBreath Dashboard</title></head><body>";
  html += "<h2>🌱 TerraBreath - Live Report</h2>";
  html += "<p><b>Soil Moisture:</b> " + String(moisture) + " %</p>";
  html += "<p><b>Soil Temperature:</b> " + String(temp) + " °C</p>";
  html += "<p><b>Soil Salinity (TDS):</b> " + String(tds) + " ppm</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

float readTemperature() {
  tempSensor.requestTemperatures();
  return tempSensor.getTempCByIndex(0);
}

float readSoilMoisture() {
  int raw = analogRead(soilMoisturePin);
  float percent = map(raw, moistureDry, moistureWet, 0, 100);
  return constrain(percent, 0, 100);
}

float readTDS(int analogValue) {
  float voltage = analogValue * (VREF / 1024.0);
  float tds = (133.42 * pow(voltage, 3)) - (255.86 * pow(voltage, 2)) + (857.39 * voltage);
  return tds;
}
