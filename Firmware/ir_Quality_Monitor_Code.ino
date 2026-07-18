#define BLYNK_TEMPLATE_ID   "Your_Template_ID"
#define BLYNK_TEMPLATE_NAME "Air Quality Monitoring System"
#define BLYNK_AUTH_TOKEN    "Your_Auth_Token"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// WiFi Credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Your_WiFi_Name";
char pass[] = "Your_WiFi_Password";

// Hardware Pin Configuration
#define SENSOR_PIN A0     // MQ-135 Gas / Air Quality Sensor
#define DHTPIN     D4     // DHT11 Sensor Pin
#define DHTTYPE    DHT11  // Sensor Type
#define RED_LED    D5     // Air Quality Hazard LED
#define GREEN_LED  D6     // Safe / Normal Air LED
#define BUZZER     D7     // Piezo Alert Alarm

// Explicit Hardware Logic Control Definitions
#define LED_ON     LOW    // Active-Low LEDs
#define LED_OFF    HIGH

#define BUZZER_ON  HIGH   // Active-High Buzzer Output
#define BUZZER_OFF LOW    

// Sensor Calibration Constants
const int AIR_BASELINE_ADC = 150; // Room baseline boundary (~198-200 ADC)
const int AIR_MAX_ADC      = 800; // Upper pollution limit

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

void processAirQualityData() {
  // 1. Read Raw MQ-135 Sensor Data
  int rawADC = analogRead(SENSOR_PIN);

  // 2. Calibrate Raw ADC to 0-500 CPCB AQI Scale
  int calculatedAQI = map(rawADC, AIR_BASELINE_ADC, AIR_MAX_ADC, 10, 500);
  if (calculatedAQI < 0) calculatedAQI = 0;
  if (calculatedAQI > 500) calculatedAQI = 500;

  // 3. Read Climate Metrics from DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // 4. Stream Telemetry to Blynk Cloud Dashboard
  Blynk.virtualWrite(V0, calculatedAQI); // Calibrated AQI
  Blynk.virtualWrite(V1, rawADC);        // Raw Voltage Count
  Blynk.virtualWrite(V2, temperature);   // Temperature (°C)
  Blynk.virtualWrite(V3, humidity);      // Humidity (%)

  // 5. LCD Row 0: Ultra-Compact Formatting (Fits 16 Characters Exactly)
  lcd.setCursor(0, 0);
  lcd.print("A:");
  lcd.print(calculatedAQI);
  if (calculatedAQI < 10) lcd.print("  ");
  else if (calculatedAQI < 100) lcd.print(" ");

  if (!isnan(temperature)) {
    lcd.print("T:");
    lcd.print((int)temperature);
    lcd.print("C ");
  }

  if (!isnan(humidity)) {
    lcd.print("H:");
    lcd.print((int)humidity);
    lcd.print("%");
  }

  // 6. LCD Row 1: CPCB Classification & Hardware Alert Logic
  lcd.setCursor(0, 1);

  if (calculatedAQI <= 50) {
    // BAND 1: Good
    lcd.print("STATUS: GOOD    ");
    digitalWrite(GREEN_LED, LED_ON);
    digitalWrite(RED_LED, LED_OFF);
    digitalWrite(BUZZER, BUZZER_OFF);
  } 
  else if (calculatedAQI <= 100) {
    // BAND 2: Satisfactory
    lcd.print("STATUS: SATISFY ");
    digitalWrite(GREEN_LED, LED_ON);
    digitalWrite(RED_LED, LED_OFF);
    digitalWrite(BUZZER, BUZZER_OFF);
  } 
  else if (calculatedAQI <= 200) {
    // BAND 3: Moderate
    lcd.print("STATUS: MODERATE");
    digitalWrite(GREEN_LED, LED_ON);
    digitalWrite(RED_LED, LED_OFF);
    digitalWrite(BUZZER, BUZZER_OFF);
  } 
  else if (calculatedAQI <= 300) {
    // BAND 4: Poor
    lcd.print("STATUS: POOR    ");
    digitalWrite(GREEN_LED, LED_OFF);
    digitalWrite(RED_LED, LED_ON);
    digitalWrite(BUZZER, BUZZER_OFF);
  } 
  else if (calculatedAQI <= 400) {
    // BAND 5: Very Poor
    lcd.print("STATUS: V. POOR ");
    digitalWrite(GREEN_LED, LED_OFF);
    digitalWrite(RED_LED, LED_ON);
    
    // Warning Beep
    digitalWrite(BUZZER, BUZZER_ON);
    delay(100);
    digitalWrite(BUZZER, BUZZER_OFF);
  } 
  else {
    // BAND 6: Severe
    lcd.print("STATUS: SEVERE! ");
    digitalWrite(GREEN_LED, LED_OFF);
    digitalWrite(RED_LED, LED_ON);
    digitalWrite(BUZZER, BUZZER_ON); // Continuous Alarm
  }
}

void setup() {
  Serial.begin(115200);

  // Pin Initialization
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Default Initial Hardware State
  digitalWrite(RED_LED, LED_OFF);
  digitalWrite(GREEN_LED, LED_OFF);
  digitalWrite(BUZZER, BUZZER_OFF);

  // Initialize Sensors & Displays
  dht.begin();
  Wire.begin(D2, D1); // SDA = D2, SCL = D1
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("  AIR QUALITY  ");
  lcd.setCursor(0, 1);
  lcd.print(" MONITOR SYSTEM ");
  delay(2000);
  lcd.clear();

  // Connect to Blynk Cloud
  Blynk.begin(auth, ssid, pass);

  // Read sensors every 1.5 seconds
  timer.setInterval(1500L, processAirQualityData);
}

void loop() {
  Blynk.run();
  timer.run();
}
