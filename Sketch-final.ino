#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>


#define BUZZER_PIN D1 // Buzzer pin (GPIO 5)
#define LIGHT_PIN D2  // Light pin (GPIO 4)
#define MQ7_SENSOR_PIN A0 // Analog pin for MQ7 sensor


const char* ssid = "B300-02C7"; // Replace with your WiFi SSID
const char* password = "e438a278";  // Replace with your WiFi password
const char* server = "192.168.103:5000";
const bool buzzerStatus = false;

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.102:5000/gas-readings";
const char* serverNameGet = "http://192.168.102:5000/get_buzz_status";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;

unsigned long timerDelay = 5000;

String sensorReadings;
float sensorReadingsArr[3];

// get request inistanciator


String httpGETRequest(const char* serverNameGet) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverNameGet);
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

// ends here


String getCO2Concentration() {
  // Read CO2 concentration from MQ7 sensor connected to A0 pin
  int co2Level = analogRead(A0);
  // Convert analog reading to ppm (replace this with your calibration formula)
  float ppmValue = co2Level / 1023.0 * 2000.0; // Example formula, replace with actual calibration
  // Convert ppm to string
  String concentration = String(ppmValue);
  return concentration;
}

String getTimeStamp() {
  // Get the current time
  unsigned long currentTime = millis();
  // Calculate the number of seconds since epoch (January 1, 1970)
  unsigned long epochSeconds = currentTime / 1000;
  // Calculate the number of seconds for the current year
  unsigned long currentYearSeconds = 365 * 24 * 60 * 60; // Number of seconds in a year
  // Calculate the number of seconds since the current year started
  unsigned long currentYearStartSeconds = epochSeconds % currentYearSeconds;
  
  // Calculate the year
  unsigned long year = 1970 + (epochSeconds / currentYearSeconds);
  
  // Calculate the number of days, hours, minutes, and seconds in the current year
  unsigned long days = currentYearStartSeconds / (24 * 60 * 60); // Number of days
  unsigned long hours = (currentYearStartSeconds % (24 * 60 * 60)) / (60 * 60); // Number of hours
  unsigned long minutes = (currentYearStartSeconds % (60 * 60)) / 60; // Number of minutes
  unsigned long seconds = currentYearStartSeconds % 60; // Number of seconds
  
  // Format the timestamp string
  char timeStampStr[20]; // Buffer to store the formatted timestamp
  sprintf(timeStampStr, "%04d-%02d-%02dT%02d:%02d:%02d", year, 1, days + 1, hours, minutes, seconds); // Month is set to 1 (January)

  return String(timeStampStr);
}


void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
    //buzzer and light
  pinMode(BUZZER_PIN, OUTPUT); // Set the buzzer pin as output
  pinMode(LIGHT_PIN, OUTPUT);  // Set the light pin as output

}

void loop() {

  int coLevel = analogRead(MQ7_SENSOR_PIN); // Read CO2 level from sensor
 // Serial.print("CO Level: ");
  //Serial.println(coLevel);


  //Send an HTTP POST request every 10 minutes
    if ((millis() - lastTime) > timerDelay) {
      //Check WiFi connection status
      if(WiFi.status()== WL_CONNECTED){
        WiFiClient client;
        HTTPClient http;
        
        // Your Domain name with URL path or IP address with path
        http.begin(client, serverName);
    
        // Specify content-type header
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        // If you need an HTTP request with a content type: application/json, use the following:
        http.addHeader("Content-Type", "application/json");
        String postData = "{";
        postData += "\"component_id\":\"1\",";
        postData += "\"status\":\"true\",";
        postData += "\"timestamp\":\"" + getTimeStamp() + "\",";
        postData += "\"concentration\":\"" + getCO2Concentration() + "\"";
        postData += "}";

        int httpResponseCode = http.POST(postData);
      
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        /// gett stuuf starts here
        sensorReadings = httpGETRequest(serverNameGet);
        //Serial.println(sensorReadings);

        JSONVar myObject = JSON.parse(sensorReadings);
  
        // JSON.typeof(jsonVar) can be used to get the type of the var
        if (JSON.typeof(myObject) == "undefined") {
          Serial.println("Parsing input failed!");
          return;
        }
    
        //Serial.print("JSON object = ");
        Serial.println(myObject["buzz_status"]);
        for(int i= 0; i<=10; i++)
        {
          if (buzzerStatus != boolean(myObject["buzz_status"]))
          {
            digitalWrite(BUZZER_PIN, HIGH); // Turn the buzzer on
            digitalWrite(LIGHT_PIN, HIGH);  // Turn the light on
            delay(100); // Wait for 100 milliseconds (adjust this value to change the buzzer/light frequency)
            digitalWrite(BUZZER_PIN, LOW); // Turn the buzzer off
            digitalWrite(LIGHT_PIN, LOW);  // Turn the light off
            delay(100); // Wait for 100 milliseconds
          }
        }

        /// gett
          
        // Free resources
        http.end();
      
      }
      else {
        Serial.println("WiFi Disconnected");
      }
        // Check if CO2 level is above 30 (adjust threshold as needed)
      if (coLevel > 850) {
        // Make the buzzer and light sound/illuminate for 30 seconds
        unsigned long startTime = millis(); // Get the start time
        while (millis() - startTime < 30000) { // Loop for 30 seconds
          digitalWrite(BUZZER_PIN, HIGH); // Turn the buzzer on
          digitalWrite(LIGHT_PIN, HIGH);  // Turn the light on
          delay(100); // Wait for 100 milliseconds (adjust this value to change the buzzer/light frequency)
          digitalWrite(BUZZER_PIN, LOW); // Turn the buzzer off
          digitalWrite(LIGHT_PIN, LOW);  // Turn the light off
          delay(100); // Wait for 100 milliseconds
        }
        // Buzzer and light will stop after 30 seconds due to loop ending
      } else {
      // CO2 level is below threshold, turn off buzzer and light
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LIGHT_PIN, LOW);

      lastTime = millis();
    }
  }
}