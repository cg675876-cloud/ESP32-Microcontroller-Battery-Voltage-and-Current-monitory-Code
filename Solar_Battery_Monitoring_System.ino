#include <WiFi.h>
#include <ThingSpeak.h>
#include <math.h>

// ==================================================
// 1. WIFI AND THINGSPEAK SETTINGS
// ==================================================

// Replace these placeholders with your own credentials.
// Do not publish real passwords or API keys on GitHub.
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

unsigned long CHANNEL_ID = 0; // Replace with your ThingSpeak Channel ID
const char* WRITE_API_KEY = "YOUR_WRITE_API_KEY";

WiFiClient client;

// ==================================================
// 2. ESP32 PIN ASSIGNMENTS
// ==================================================

const int VOLTAGE_PIN = 34;
const int CURRENT_PIN = 35;

const int BUZZER_PIN = 25;
const int RELAY_PIN = 26;

const int YELLOW_LED_PIN = 32;
const int RED_LED_PIN = 33;
const int BLUE_LED_PIN = 27;

// ==================================================
// 3. BATTERY SETTINGS
// ==================================================

const float NOMINAL_BATTERY_VOLTAGE = 11.10;

const float FULL_BATTERY_VOLTAGE = 12.60;
const float RESTART_CHARGING_VOLTAGE = 12.20;

const float LOW_BATTERY_VOLTAGE = 10.50;

// Voltage sensor calibration
const float VOLTAGE_SENSOR_RATIO = 5.00;

// Voltage graph display scale
const float VOLTAGE_GRAPH_MAX = 15.00;

// ACS712 5A calibration
const float ACS712_SENSITIVITY = 0.185;
const float ACS712_ZERO_VOLTAGE = 2.60;

const float CURRENT_DEADBAND = 0.08;

// ==================================================
// 4. RELAY AND BUZZER CONFIGURATION
// ==================================================

const bool RELAY_ACTIVE_LOW = true;
const bool BUZZER_ACTIVE_HIGH = true;

// ==================================================
// 5. TIMING SETTINGS
// ==================================================

const unsigned long UPLOAD_INTERVAL = 20000;
const unsigned long BLINK_INTERVAL = 500;

const unsigned long BUZZER_DURATION = 300;
const unsigned long WIFI_RECONNECT_INTERVAL = 10000;

unsigned long lastUploadTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastReconnectTime = 0;

unsigned long buzzerStartTime = 0;

// ==================================================
// 6. SYSTEM VARIABLES
// ==================================================

bool chargingEnabled = false;
bool batteryFull = false;

bool redBlinkState = false;

bool chargingWarningActive = false;
bool chargingWarningDone = false;

// ==================================================
// 7. RELAY CONTROL
// ==================================================

void setChargerRelay(bool connectCharger) {

  if (RELAY_ACTIVE_LOW) {

    digitalWrite(RELAY_PIN,
                 connectCharger ? LOW : HIGH);

  } else {

    digitalWrite(RELAY_PIN,
                 connectCharger ? HIGH : LOW);
  }
}

// ==================================================
// 8. BUZZER CONTROL
// ==================================================

void setBuzzer(bool alarmOn) {

  if (BUZZER_ACTIVE_HIGH) {

    digitalWrite(BUZZER_PIN,
                 alarmOn ? HIGH : LOW);

  } else {

    digitalWrite(BUZZER_PIN,
                 alarmOn ? LOW : HIGH);
  }
}

// ==================================================
// 9. VOLTAGE MEASUREMENT
// ==================================================

float readBatteryVoltage() {

  uint32_t totalMilliVolts = 0;

  for (int i = 0; i < 20; i++) {

    totalMilliVolts +=
        analogReadMilliVolts(VOLTAGE_PIN);

    delay(2);
  }

  float sensorVoltage =
      (totalMilliVolts / 20.0) / 1000.0;

  return sensorVoltage * VOLTAGE_SENSOR_RATIO;
}

// ==================================================
// 10. CURRENT MEASUREMENT
// ==================================================

float readBatteryCurrent() {

  uint32_t totalMilliVolts = 0;

  for (int i = 0; i < 50; i++) {

    totalMilliVolts +=
        analogReadMilliVolts(CURRENT_PIN);

    delay(2);
  }

  float sensorVoltage =
      (totalMilliVolts / 50.0) / 1000.0;

  float current =
      (sensorVoltage - ACS712_ZERO_VOLTAGE)
      / ACS712_SENSITIVITY;

  if (fabs(current) < CURRENT_DEADBAND) {

    current = 0.0;
  }

  return current;
}

// ==================================================
// 11. BATTERY STATE OF CHARGE
// ==================================================

float calculateSOC(float voltage) {

  float soc =
      ((voltage - LOW_BATTERY_VOLTAGE) /
       (FULL_BATTERY_VOLTAGE - LOW_BATTERY_VOLTAGE))
      * 100.0;

  if (soc > 100.0) soc = 100.0;

  if (soc < 0.0) soc = 0.0;

  return soc;
}

// ==================================================
// 12. CHARGING CONTROL
// ==================================================

void updateBatteryControl(float voltage) {

  // Stop charging at 12.60 V.

  if (voltage >= FULL_BATTERY_VOLTAGE) {

    batteryFull = true;

    chargingEnabled = false;

    chargingWarningActive = false;

    chargingWarningDone = false;

    setBuzzer(false);
  }

  // Restart charging at or below 12.20 V.

  else if (voltage <= RESTART_CHARGING_VOLTAGE) {

    if (!chargingEnabled && !chargingWarningDone) {

      batteryFull = false;

      chargingWarningActive = true;

      chargingWarningDone = true;

      buzzerStartTime = millis();

      setBuzzer(true);
    }
  }

  // Pre-charge buzzer operation.

  if (chargingWarningActive) {

    if (millis() - buzzerStartTime >= BUZZER_DURATION) {

      setBuzzer(false);

      chargingWarningActive = false;

      chargingEnabled = true;
    }
  }

  // Relay control.

  setChargerRelay(chargingEnabled);

  // YELLOW LED: 11.10 V or below.

  digitalWrite(YELLOW_LED_PIN,
               voltage <= NOMINAL_BATTERY_VOLTAGE
               ? HIGH : LOW);

  // BLUE LED: 12.20 V or above.

  digitalWrite(BLUE_LED_PIN,
               voltage >= RESTART_CHARGING_VOLTAGE
               ? HIGH : LOW);

  // RED LED indication.

  if (batteryFull) {

    digitalWrite(RED_LED_PIN, HIGH);

  } else if (chargingEnabled) {

    if (millis() - lastBlinkTime >= BLINK_INTERVAL) {

      lastBlinkTime = millis();

      redBlinkState = !redBlinkState;
    }

    digitalWrite(RED_LED_PIN,
                 redBlinkState ? HIGH : LOW);

  } else {

    redBlinkState = false;

    digitalWrite(RED_LED_PIN, LOW);
  }
}

// ==================================================
// 13. WIFI CONNECTION
// ==================================================

void connectWiFi() {

  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startAttempt < 20000) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WiFi connected.");

    Serial.print("IP address: ");

    Serial.println(WiFi.localIP());

  } else {

    Serial.println("WiFi connection failed.");
  }
}

// ==================================================
// 14. THINGSPEAK UPLOAD
// ==================================================

void uploadToThingSpeak(float voltage,
                        float current,
                        float power,
                        float soc,
                        const char* status) {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi unavailable; upload skipped.");

    return;
  }

  float inputCurrent = 0.0;

  float outputCurrent = 0.0;

  if (current > CURRENT_DEADBAND) {

    inputCurrent = current;
  }

  if (current < -CURRENT_DEADBAND) {

    outputCurrent = -current;
  }

  ThingSpeak.setField(1, voltage);

  ThingSpeak.setField(2, inputCurrent);

  ThingSpeak.setField(3, outputCurrent);

  ThingSpeak.setField(4, power);

  ThingSpeak.setField(5, soc);

  ThingSpeak.setStatus(status);

  int result =
      ThingSpeak.writeFields(CHANNEL_ID, WRITE_API_KEY);

  if (result == 200) {

    Serial.println("ThingSpeak upload successful.");

  } else {

    Serial.print("ThingSpeak upload failed. Code: ");

    Serial.println(result);
  }
}

// ==================================================
// 15. SETUP
// ==================================================

void setup() {

  Serial.begin(115200);

  pinMode(VOLTAGE_PIN, INPUT);

  pinMode(CURRENT_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(YELLOW_LED_PIN, OUTPUT);

  pinMode(RED_LED_PIN, OUTPUT);

  pinMode(BLUE_LED_PIN, OUTPUT);

  analogReadResolution(12);

  // Initial safe state.

  setChargerRelay(false);

  setBuzzer(false);

  digitalWrite(YELLOW_LED_PIN, LOW);

  digitalWrite(RED_LED_PIN, LOW);

  digitalWrite(BLUE_LED_PIN, LOW);

  ThingSpeak.begin(client);

  connectWiFi();

  Serial.println();

  Serial.println("SOLAR BATTERY MONITORING SYSTEM");

  Serial.println("ESP32 Dev Module started.");

  Serial.print("Voltage graph maximum: ");

  Serial.print(VOLTAGE_GRAPH_MAX);

  Serial.println(" V");
}

// ==================================================
// 16. MAIN LOOP
// ==================================================

void loop() {

  float batteryVoltage = readBatteryVoltage();

  float batteryCurrent = readBatteryCurrent();

  float batteryPower =
      batteryVoltage * fabs(batteryCurrent);

  float batterySOC =
      calculateSOC(batteryVoltage);

  updateBatteryControl(batteryVoltage);

  const char* systemStatus;

  if (batteryFull) {

    systemStatus = "BATTERY FULL - CHARGER OFF";

  } else if (chargingWarningActive) {

    systemStatus = "CHARGING WARNING";

  } else if (chargingEnabled) {

    systemStatus = "CHARGING";

  } else if (batteryVoltage <= LOW_BATTERY_VOLTAGE) {

    systemStatus = "LOW BATTERY";

  } else if (batteryCurrent < -CURRENT_DEADBAND) {

    systemStatus = "DISCHARGING";

  } else {

    systemStatus = "STANDBY";
  }

  // SERIAL MONITOR DISPLAY.

  Serial.println("--------------------------------");

  Serial.print("Battery voltage: ");

  Serial.print(batteryVoltage, 2);

  Serial.println(" V");

  Serial.print("Signed current: ");

  Serial.print(batteryCurrent, 2);

  Serial.println(" A");

  Serial.print("Battery power: ");

  Serial.print(batteryPower, 2);

  Serial.println(" W");

  Serial.print("Estimated SOC: ");

  Serial.print(batterySOC, 1);

  Serial.println(" %");

  Serial.print("Charger relay: ");

  Serial.println(
      chargingEnabled ? "CONNECTED" : "DISCONNECTED"
  );

  Serial.print("System status: ");

  Serial.println(systemStatus);

  // THINGSPEAK UPLOAD EVERY 20 SECONDS.

  if (millis() - lastUploadTime >= UPLOAD_INTERVAL) {

    lastUploadTime = millis();

    uploadToThingSpeak(
        batteryVoltage,
        batteryCurrent,
        batteryPower,
        batterySOC,
        systemStatus
    );
  }

  // WIFI RECONNECTION.

  if (WiFi.status() != WL_CONNECTED &&
      millis() - lastReconnectTime >= WIFI_RECONNECT_INTERVAL) {

    lastReconnectTime = millis();

    Serial.println("WiFi disconnected. Reconnecting...");

    WiFi.reconnect();
  }

  delay(200);
}
