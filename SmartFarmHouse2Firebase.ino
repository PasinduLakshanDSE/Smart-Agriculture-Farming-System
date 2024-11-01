#define BLYNK_TEMPLATE_ID "TMPL6m543lb7O"
#define BLYNK_TEMPLATE_NAME "Smart Farm House2"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h>  // Include Firebase library

// Firebase setup
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Wi-Fi credentials for Blynk and Firebase
char ssid[] = "iPhone7";     // Your WiFi network name
char pass[] = "2003013028";      // Your WiFi password
char auth[] = "hqakMZrnRFj9F2XJuD0Eh7CgSLBVYuLB"; // Declare the Blynk auth token

// Firebase project details
#define API_KEY "AIzaSyAdyM-2se9oH74elMxnCo3JsuCpFE-wNJQ"
#define DATABASE_URL "dht11-7b405-default-rtdb.firebaseio.com"  // Without "https://"

FirebaseData fbdo;  // Firebase data object
FirebaseAuth auth_firebase;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

BlynkTimer timer;
bool waterRelay = 0;
bool fanRelay = 0;

// Define component pins
#define waterPump D3
#define fan D5               // Define the pin for fan relay
#define soilMoisturePin A0    // Analog pin for the soil moisture sensor
#define DHTPIN D4             // Pin where DHT11 is connected
#define DHTTYPE DHT11         // DHT 11 sensor type
#define SDA_PIN D6            // Custom SDA pin for I2C
#define SCL_PIN D7            // Custom SCL pin for I2C

DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT sensor
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD I2C address to 0x27 (change if needed)

void setup() {
  Serial.begin(9600); // Initialize Serial Monitor
  pinMode(waterPump, OUTPUT);
  pinMode(fan, OUTPUT);  // Set the fan pin as output
  digitalWrite(waterPump, HIGH);  // Ensure water pump is off initially
  digitalWrite(fan, HIGH);        // Ensure fan is off initially

  dht.begin();  // Initialize DHT11 sensor

  // Initialize I2C with custom SDA and SCL pins
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.begin(16, 2, LCD_5x8DOTS);         // Initialize the LCD
  lcd.backlight();    // Turn on the backlight for the LCD

  Blynk.begin(auth, ssid, pass);

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Anonymous sign-in to Firebase
  if (Firebase.signUp(&config, &auth_firebase, "", "")) {
    Serial.println("Sign-up OK");
    signupOK = true;
  } else {
    Serial.printf("Sign-up Error: %s\n", config.signer.signupError.message.c_str());
  }

  // Token status callback
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth_firebase);
  Firebase.reconnectWiFi(true);

  // Set a timer to read sensor data every 5 seconds
  timer.setInterval(5000L, readSensors);

  Serial.println("Blynk connected and ready");
  lcd.setCursor(0, 0);
  lcd.print("Smart Farm Ready");
}

// Function to read soil moisture and DHT11 data
void readSensors() {
  // Read soil moisture data
  int soilMoistureValue = analogRead(soilMoisturePin); // Read analog value
  int moisturePercent = map(soilMoistureValue, 1023, 0, 0, 100); // Convert to percentage
  Blynk.virtualWrite(V1, moisturePercent); // Send the moisture percentage to Blynk app

  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  // Read temperature and humidity data from DHT11
  float humidity = dht.readHumidity();    // Read humidity
  float temperature = dht.readTemperature();  // Read temperature (Celsius)

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V2, temperature); // Send temperature to Blynk app
  Blynk.virtualWrite(V3, humidity);    // Send humidity to Blynk app

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  // Display Temperature, Humidity, and Soil Moisture on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);  // Set cursor to the first row
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);  // Set cursor to the second row
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");

  // Firebase integration to push data to the Firebase Realtime Database
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();  // Update timestamp

    // Create JSON object to store the readings
    FirebaseJson json;
    json.set("Temperature", temperature);
    json.set("Humidity", humidity);
    json.set("SoilMoisture", moisturePercent);  // Add soil moisture data to Firebase

    // Push data to Firebase
    if (Firebase.RTDB.pushJSON(&fbdo, "SMART_FARM_HOUSE/readings", &json)) {
      Serial.println("Data pushed successfully to Firebase");
    } else {
      Serial.println("Failed to push data to Firebase");
      Serial.println(fbdo.errorReason());
    }
  }
}

// Get the button value from Blynk app to control water pump
BLYNK_WRITE(V0) {
  waterRelay = param.asInt();

  if (waterRelay == 1) {
    digitalWrite(waterPump, LOW);  // Turn on water pump
    Serial.println("Water pump ON");
  } else {
    digitalWrite(waterPump, HIGH); // Turn off water pump
    Serial.println("Water pump OFF");
  }
}

// Get the button value from Blynk app to control fan
BLYNK_WRITE(V4) {
  fanRelay = param.asInt();

  if (fanRelay == 1) {
    digitalWrite(fan, LOW);  // Turn on fan
    Serial.println("Fan ON");
  } else {
    digitalWrite(fan, HIGH); // Turn off fan
    Serial.println("Fan OFF");
  }
}

void loop() {
  Blynk.run();  // Run the Blynk library
  timer.run();  // Run the Blynk timer
}
