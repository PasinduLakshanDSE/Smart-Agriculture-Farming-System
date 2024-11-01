#define BLYNK_TEMPLATE_ID "TMPL6Co72S-XU"
#define BLYNK_TEMPLATE_NAME "Smart Farm House"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>  // Include Servo library

// Blynk settings
char ssid[] = "iPhone7";     // Your WiFi network name
char pass[] = "2003013028";      // Your WiFi password
char auth[] = "_e0wjn_coM4RHfUcyAPYrsuf3TFYtLgm"; // Declare the auth token

#define MOTION_EVENT "motionnotification"
#define RAIN_EVENT "raiinnotification"

// PIR motion sensor settings
#define PIR_SENSOR_PIN D5     // PIR output connected to GPIO14 (D5)

// Rain sensor settings (using analog pin A0)
#define RAIN_SENSOR_PIN A0    // Rain sensor connected to the A0 pin

// Servo motor settings
#define SERVO_PIN1 D4
#define SERVO_PIN2 D7  
Servo servo1;  // Create Servo object for the first servo
Servo servo2;  // Create Servo object for the second servo

// LED bulb settings
#define LED_PIN D3   // Define the pin connected to the LED bulb (GPIO 0 or D3)

void setup() {
  delay(1000);               // Give some time for initialization
  Serial.begin(115200);      // Start serial communication at 115200 baud

  pinMode(PIR_SENSOR_PIN, INPUT);  // Set PIR sensor pin as input
  pinMode(LED_PIN, OUTPUT);        // Set LED pin as output

  // Initialize servo motors
  servo1.attach(SERVO_PIN1);
  servo2.attach(SERVO_PIN2);  // Attach the second servo to its specified pin
  servo1.write(0);           // Start with the first servo in position 0
  servo2.write(0);           // Start with the second servo in position 0

  Serial.println("Smart Farm Init");
  delay(2000);

  // Blynk connection
  Blynk.begin(auth, ssid, pass);
}

// Blynk function to control servo1 via virtual pin V5
BLYNK_WRITE(V5)  
{
  int pinValue = param.asInt();  // Get the state of the virtual pin

  if (pinValue == 1) {
    servo1.write(180);  // Rotate the first servo to 180 degrees
    Serial.println("Servo1 ON - 180 degrees");
  } else {
    servo1.write(0);  // Return the first servo to 0 degrees
    Serial.println("Servo1 OFF - 0 degrees");
  }
}

// Blynk function to control servo2 via virtual pin V1
BLYNK_WRITE(V1)  
{
  int pinValue = param.asInt();  // Get the state of the virtual pin

  if (pinValue == 1) {
    servo2.write(180);  // Rotate the second servo to 180 degrees
    Serial.println("Servo2 ON - 180 degrees");
  } else {
    servo2.write(0);  // Return the second servo to 0 degrees
    Serial.println("Servo2 OFF - 0 degrees");
  }
}

// Blynk function to control the LED bulb via virtual pin V2
BLYNK_WRITE(V2)  
{
  int ledState = param.asInt();  // Get the state of the virtual pin

  if (ledState == 1) {
    digitalWrite(LED_PIN, HIGH);  // Turn on the LED
    Serial.println("LED ON");
  } else {
    digitalWrite(LED_PIN, LOW);  // Turn off the LED
    Serial.println("LED OFF");
  }
}

void loop() {
  Blynk.run();  // Run Blynk

  delay(2000);  // Wait 2 seconds between readings

  // Read PIR motion sensor value (HIGH = motion detected, LOW = no motion)
  int pirValue = digitalRead(PIR_SENSOR_PIN);

  // Check for motion detection
  if (pirValue == HIGH) {
    Serial.println("Motion detected!");
    Blynk.virtualWrite(V3, 1);  // Send motion detection signal to Blynk
    Blynk.logEvent(MOTION_EVENT, "Motion Detected in the Farm Main Gate");  // Log motion event in Blynk
  } else {
    Serial.println("No motion.");
    Blynk.virtualWrite(V3, 0);  // Send no motion signal to Blynk
  }

  // Read rain sensor value (0 to 1023, where lower values mean more rain)
  int rainValue = analogRead(RAIN_SENSOR_PIN);

  // Check for rain detection (arbitrary threshold value for rain detection)
  if (rainValue > 300) {  // Adjust the threshold based on your sensor's calibration
    Serial.println("Rain detected!");
    Blynk.virtualWrite(V0, 1);  // Send rain signal to Blynk
    Blynk.logEvent(RAIN_EVENT, "Rain Detected! Please Close the Chamber Door");  // Log rain event in Blynk
  } else {
    Serial.println("No rain.");
    Blynk.virtualWrite(V0, 0);  // Send no rain signal to Blynk
  }
}
