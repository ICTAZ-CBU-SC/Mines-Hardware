#include <ESP8266WiFi.h>

const char* ssid = "B300-02C7"; // Replace with your WiFi SSID
const char* pass = "e438a278";  // Replace with your WiFi password
const char* server = "192.168.103:5000";
const String apiKey = "7KXJUIH0ONV5YDSO"; // Enter your ThingSpeak API Key

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  float h = analogRead(A0);

  if (isnan(h)) {
    Serial.println("Failed to read from MQ-7 sensor!");
    return;
  }

  if (client.connect(server, 80)) {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(h / 1023 * 100);
    postStr += "\r\n";

    client.print("POST /gas-readings HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: \n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    
    Serial.print("Gas Level: ");
    Serial.println(h / 1023 * 100);
    Serial.println("Data Sent to ThingSpeak");
  }
  
  delay(500);
  client.stop();
  delay(15000); // Thingspeak needs minimum 15 sec delay between updates
}
