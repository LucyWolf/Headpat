#include <Arduino.h>
#include <bluefruit.h>
#include <nrf_gpio.h>
#include <nrf_power.h>

// ==========================================================
// DEBUG
// ==========================================================

#define DEBUG_ENABLED 1
#define DEBUG_HAPTIC  0

#if DEBUG_ENABLED
  #define DEBUG_BEGIN(x) Serial.begin(x)
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_BEGIN(x)
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// ==========================================================
// PINS
// ==========================================================

#define MOTOR_LEFT   43
#define MOTOR_RIGHT  10
#define LED_PIN      15
#define BUTTON_PIN    6

// ==========================================================
// MOTOR LOGIK
// ==========================================================

#define HAPTIC_ON  HIGH
#define HAPTIC_OFF LOW

// ==========================================================
// BLE
// ==========================================================

BLEUart bleuart;
bool connected = false;

// ==========================================================
// BUTTON
// ==========================================================

unsigned long buttonPressStart = 0;
bool buttonHeld = false;
bool buttonWasReleased = true;

// ==========================================================
// LED
// ==========================================================

unsigned long lastBlink = 0;

// ==========================================================
// KEEPALIVE OHNE AKKU
// ==========================================================

unsigned long lastBatteryUpdate = 0;

// ==========================================================
// MOTOR AUS
// ==========================================================

void motorOff() {
  digitalWrite(MOTOR_LEFT, HAPTIC_OFF);
  digitalWrite(MOTOR_RIGHT, HAPTIC_OFF);
}

// ==========================================================
// STARTUP VIBRATION
// ==========================================================

void startupVibration() {
  DEBUG_PRINTLN("[STARTUP] Vibration");

  digitalWrite(MOTOR_LEFT, HAPTIC_ON);
  digitalWrite(MOTOR_RIGHT, HAPTIC_ON);
  delay(800);

  motorOff();
  delay(150);

  digitalWrite(MOTOR_LEFT, HAPTIC_ON);
  digitalWrite(MOTOR_RIGHT, HAPTIC_ON);
  delay(800);

  motorOff();
  delay(150);

  digitalWrite(MOTOR_LEFT, HAPTIC_ON);
  digitalWrite(MOTOR_RIGHT, HAPTIC_ON);
  delay(1000);

  motorOff();
}

// ==========================================================
// SHUTDOWN ANIMATION
// ==========================================================

void shutdownAnimation() {
  DEBUG_PRINTLN("[POWER] Shutdown animation");

  digitalWrite(LED_PIN, HIGH);
  delay(100);

  for (int i = 255; i >= 0; i -= 3) {
    analogWrite(LED_PIN, i);
    delay(10);
  }

  analogWrite(LED_PIN, 0);
}

// ==========================================================
// SLEEP MODE
// ==========================================================

void goToSleep() {
  DEBUG_PRINTLN("[POWER] Going to sleep");

  motorOff();
  digitalWrite(LED_PIN, LOW);

  shutdownAnimation();

  nrf_gpio_cfg_sense_input(BUTTON_PIN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  sd_power_system_off();
}

// ==========================================================
// BLE CALLBACKS
// ==========================================================

void connect_callback(uint16_t conn_handle) {
  connected = true;
  digitalWrite(LED_PIN, LOW);

  DEBUG_PRINTLN("[BLE] Connected");
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  connected = false;
  motorOff();

  DEBUG_PRINT("[BLE] Disconnected, reason=");
  DEBUG_PRINTLN(reason);
}

// ==========================================================
// SETUP
// ==========================================================

void setup() {
  DEBUG_BEGIN(115200);
  delay(200);

  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("=================================");
  DEBUG_PRINTLN("HeatPett nRF52840 Booting");
  DEBUG_PRINTLN("=================================");

  pinMode(MOTOR_LEFT, OUTPUT);
  pinMode(MOTOR_RIGHT, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  motorOff();
  digitalWrite(LED_PIN, LOW);

  startupVibration();

  DEBUG_PRINTLN("[BLE] Starting Bluefruit");

  Bluefruit.begin();
  Bluefruit.setName("HeatPett");

  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  bleuart.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.start(0);

  DEBUG_PRINTLN("[BLE] Advertising started");
}

// ==========================================================
// MOTOR CONTROL
// ==========================================================

void handleHaptics() {
  if (!connected) {
    motorOff();
    return;
  }

  while (bleuart.available() > 0) {
    char data = bleuart.read();

    unsigned int haptic_right_level = (data & 0x0F) << 4;
    unsigned int haptic_left_level  = data & 0xF0;

    haptic_right_level |= haptic_right_level >> 4;
    haptic_left_level  |= haptic_left_level >> 4;

    analogWrite(MOTOR_LEFT,  HAPTIC_OFF ? haptic_left_level  : 0xFF - haptic_left_level);
    analogWrite(MOTOR_RIGHT, HAPTIC_OFF ? haptic_right_level : 0xFF - haptic_right_level);

#if DEBUG_HAPTIC
    DEBUG_PRINT("[HAPTIC] L=");
    DEBUG_PRINT(haptic_left_level);
    DEBUG_PRINT(" R=");
    DEBUG_PRINTLN(haptic_right_level);
#endif
  }
}

// ==========================================================
// KEEPALIVE
// ==========================================================

void handleKeepAlive() {
  if (connected && millis() - lastBatteryUpdate >= 1000) {
    lastBatteryUpdate = millis();

    // Kein Akku angeschlossen: 100% senden
    bleuart.write((uint8_t)100);

    DEBUG_PRINTLN("[BLE] Keepalive sent: 100");
  }
}

// ==========================================================
// BUTTON
// ==========================================================

void handleButton() {
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (buttonPressed) {
    digitalWrite(LED_PIN, HIGH);

    if (buttonWasReleased) {
      buttonPressStart = millis();
      buttonWasReleased = false;
      buttonHeld = false;

      DEBUG_PRINTLN("[BUTTON] Pressed");
    }

    if (!buttonHeld && millis() - buttonPressStart >= 5000) {
      buttonHeld = true;

      DEBUG_PRINTLN("[BUTTON] Long press detected");
      goToSleep();
    }

  } else {
    if (!buttonWasReleased) {
      DEBUG_PRINTLN("[BUTTON] Released");
    }

    buttonWasReleased = true;
    buttonHeld = false;

    if (connected) {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

// ==========================================================
// STATUS LED
// ==========================================================

void handleLedBlink() {
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  if (!connected && !buttonPressed) {
    motorOff();

    unsigned long now = millis();

    if (now - lastBlink >= 3000) {
      lastBlink = now;

      digitalWrite(LED_PIN, HIGH);
      delay(80);
      digitalWrite(LED_PIN, LOW);
    }
  }
}

// ==========================================================
// LOOP
// ==========================================================

void loop() {
  handleHaptics();
  handleKeepAlive();
  handleButton();
  handleLedBlink();
}