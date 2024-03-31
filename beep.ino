#define BUZZER_PIN 2 // Define the pin connected to the buzzer (GPIO 2)

void setup() {
  pinMode(BUZZER_PIN, OUTPUT); // Set the buzzer pin as output
}

void loop() {
  // Make the buzzer sound for 30 seconds
  unsigned long startTime = millis(); // Get the start time
  while (millis() - startTime < 30000) { // Loop for 30 seconds
    digitalWrite(BUZZER_PIN, HIGH); // Turn the buzzer on
    delay(100); // Wait for 100 milliseconds (adjust this value to change the buzzer frequency)
    digitalWrite(BUZZER_PIN, LOW); // Turn the buzzer off
    delay(100); // Wait for 100 milliseconds
  }
  // Buzzer will stop after 30 seconds due to loop ending
}