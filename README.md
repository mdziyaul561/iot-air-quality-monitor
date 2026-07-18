# iot-air-quality-monitor
An IoT embedded system using NodeMCU, MQ-135, and DHT11 with Blynk cloud integration
## 🛠️ Hardware Components
* ESP8266 NodeMCU Microcontroller
* MQ-135 Gas Sensor
* DHT11 Temperature & Humidity Sensor
* 16x2 I2C LCD Display Module
* Alert Indicators (Green/Red LEDs & Active Buzzer)

## 📊 Core Features
* **CPCB AQI Mapping:** Calibrates raw 10-bit analog voltage data into standard Central Pollution Control Board categories (0-500 scale).
* **Local Hardware Alarms:** Instantly triggers an acoustic buzzer and switches to a Red LED hazard state if the AQI exceeds 200.
* **Cloud Telemetry:** Live updates stream to a mobile Blynk IoT dashboard via Wi-Fi virtual pins.
