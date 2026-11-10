
#include "HomeSpan.h"

// ========================================
// Hardware configuration
// ========================================

#define BUZZER_PIN 7
#define BUTTON_PIN 6

// Passive buzzer tone
#define BUZZER_FREQUENCY 2000
#define BUZZER_RESOLUTION 8
#define BUZZER_DUTY 128  // 50% duty cycle

// Button debounce
#define DEBOUNCE_DELAY 50

// ========================================
// Global HomeKit characteristic
// ========================================

SpanCharacteristic *buzzerPower = nullptr;

// ========================================
// Buzzer control
// ========================================

void setBuzzer(bool state) {

  if (state) {
    // Generate 2 kHz PWM tone
    ledcWrite(BUZZER_PIN, BUZZER_DUTY);

    Serial.println("[BUZZER] ON - 2000 Hz");

  } else {
    // Stop PWM output
    ledcWrite(BUZZER_PIN, 0);

    Serial.println("[BUZZER] OFF");
  }
}

// ========================================
// HomeKit Switch Service
// ========================================

struct BuzzerSwitch : Service::Switch {

  SpanCharacteristic *power;

  BuzzerSwitch() : Service::Switch() {

    power = new Characteristic::On(false);

    // Keep a global reference for physical button
    buzzerPower = power;

    Serial.println("[HOMEKIT] Buzzer Switch initialized");
  }

  boolean update() override {

    bool newState = power->getNewVal<bool>();

    setBuzzer(newState);

    Serial.printf(
      "[HOMEKIT] Switch changed: %s\n",
      newState ? "ON" : "OFF"
    );

    return true;
  }
};

// ========================================
// Physical button handling
// ========================================

bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;

unsigned long lastDebounceTime = 0;

void handlePhysicalButton() {

  bool reading = digitalRead(BUTTON_PIN);

  // Detect input change
  if (reading != lastButtonReading) {

    lastDebounceTime = millis();

  }

  // Debounce
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {

    if (reading != stableButtonState) {

      stableButtonState = reading;

      // Button pressed (INPUT_PULLUP = LOW)
      if (stableButtonState == LOW) {

        if (buzzerPower != nullptr) {

          bool currentState = buzzerPower->getVal<bool>();

          bool newState = !currentState;

          // Update HomeKit and notify connected controllers
          buzzerPower->setVal(newState);

          // Apply hardware change immediately
          setBuzzer(newState);

          Serial.printf(
            "[BUTTON] Buzzer toggled: %s\n",
            newState ? "ON" : "OFF"
          );
        }
      }
    }
  }

  lastButtonReading = reading;
}

// ========================================
// Setup
// ========================================

void setup() {

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("================================");
  Serial.println(" ESP32 Super Mini HomeKit Buzzer");
  Serial.println("================================");

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Configure hardware PWM
  if (!ledcAttach(
        BUZZER_PIN,
        BUZZER_FREQUENCY,
        BUZZER_RESOLUTION
      )) {

    Serial.println("[ERROR] Failed to initialize PWM!");

    while (true) {
      delay(1000);
    }
  }

  // Ensure buzzer starts OFF
  setBuzzer(false);

  // ======================================
  // Initialize HomeSpan
  // ======================================

  homeSpan.begin(
    Category::Switches,
    "ESP32 Buzzer"
  );

  // Create HomeKit accessory
  new SpanAccessory();

    // Accessory information
    new Service::AccessoryInformation();

      new Characteristic::Identify();

    // Buzzer switch
    new BuzzerSwitch();

  Serial.println("[SYSTEM] Initialization complete");

}

// ========================================
// Main loop
// ========================================

void loop() {

  // Process HomeKit communication
  homeSpan.poll();

  // Read physical button
  handlePhysicalButton();

}
