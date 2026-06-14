#include <Arduino.h>
#include <bluefruit.h>
#include <nrf_gpio.h>
#include <nrf_power.h>

#define VERSION "1.0.1"

// ═══════════════════════════════════════════
//  PINS
// ═══════════════════════════════════════════
#define MOTOR_LEFT   43
#define MOTOR_RIGHT  10
#define LED_PIN      15
#define BUTTON_PIN    6

// ═══════════════════════════════════════════
//  EINSTELLUNGEN
// ═══════════════════════════════════════════
#define HAPTIC_ON            HIGH
#define HAPTIC_OFF           LOW
#define PAIRING_HOLD_MS      3000
#define SLEEP_HOLD_MS        5000
#define PAIRING_DURATION_MS  20000
#define PAIRING_BLINK_MS     150
#define KEEPALIVE_MS         1000
#define IDLE_BLINK_MS        3000

int motorStrength = 255;

// ═══════════════════════════════════════════
//  GLOBALE VARIABLEN
// ═══════════════════════════════════════════
BLEUart bleuart;

bool connected         = false;
bool pairingMode       = false;
bool buttonWasReleased = true;
bool sleepTriggered    = false;

unsigned long buttonPressStart = 0;
unsigned long lastKeepAlive    = 0;
unsigned long pairingStart     = 0;
unsigned long lastPairingBlink = 0;
unsigned long lastIdleBlink    = 0;

bool pairingLedState = false;

// ═══════════════════════════════════════════
//  MOTOR
// ═══════════════════════════════════════════
void stopMotors() {
  digitalWrite(MOTOR_LEFT, HAPTIC_OFF);
  digitalWrite(MOTOR_RIGHT, HAPTIC_OFF);
  analogWrite(MOTOR_LEFT, 0);
  analogWrite(MOTOR_RIGHT, 0);
}

void startupVibration() {
  Serial.println("[VIBRATION] Startup START");

  analogWrite(MOTOR_LEFT, motorStrength);
  analogWrite(MOTOR_RIGHT, motorStrength);
  delay(1000);

  stopMotors();

  Serial.println("[VIBRATION] Startup DONE");
}

// ═══════════════════════════════════════════
//  PAIRING
// ═══════════════════════════════════════════
void startPairingMode() {
  Serial.println("[PAIRING] Mode START");
  pairingMode = true;
  pairingStart = millis();
  Bluefruit.Advertising.start(0);
}

void stopPairingMode() {
  Serial.println("[PAIRING] Mode STOP");
  pairingMode = false;
  digitalWrite(LED_PIN, LOW);

  if (!connected) {
    Bluefruit.Advertising.stop();
  }
}

// ═══════════════════════════════════════════
//  SLEEP
// ═══════════════════════════════════════════
void goToSleep() {
  Serial.println("[POWER] Going to sleep...");
  delay(100);

  stopMotors();
  digitalWrite(LED_PIN, LOW);

  pairingMode = false;
  Bluefruit.Advertising.stop();

  delay(200);

  nrf_gpio_cfg_sense_input(
    BUTTON_PIN,
    NRF_GPIO_PIN_PULLUP,
    NRF_GPIO_PIN_SENSE_LOW
  );

  Serial.println("[POWER] SYSTEM OFF");
  delay(100);

  sd_power_system_off();

  NRF_POWER->SYSTEMOFF = 1;
  while (1) {}
}

// ═══════════════════════════════════════════
//  BLE CALLBACKS
// ═══════════════════════════════════════════
void connect_callback(uint16_t conn_handle) {
  Serial.println("[BLE] Connected!");
  connected = true;
  pairingMode = false;
  digitalWrite(LED_PIN, LOW);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.print("[BLE] Disconnected! Reason=");
  Serial.println(reason);

  connected = false;
  stopMotors();
}

// ═══════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("══════════════════════════════");
  Serial.print("  HeatPett v");
  Serial.println(VERSION);
  Serial.println("══════════════════════════════");

  pinMode(MOTOR_LEFT, OUTPUT);
  pinMode(MOTOR_RIGHT, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  stopMotors();
  digitalWrite(LED_PIN, LOW);

  Serial.print("[SYS] Motor strength: ");
  Serial.println(motorStrength);
  Serial.println("[SYS] Command: s=255");

  startupVibration();

  Bluefruit.begin();
  Bluefruit.setName("HeatPett");

  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  bleuart.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(false);
  Bluefruit.Advertising.start(0);

  Serial.println("[BLE] Advertising started");
  Serial.println("[SYS] Ready!");
}

// ═══════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════
void loop() {

  // ── Serial Commands ─────────────────────
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("s=")) {
      int val = cmd.substring(2).toInt();
      motorStrength = constrain(val, 0, 255);

      Serial.print("[SYS] Motor strength set to: ");
      Serial.println(motorStrength);
    }
  }

  // ── Motoren nur bei BLE Verbindung ──────
  if (connected) {
    while (bleuart.available() > 0) {
      char data = bleuart.read();

      unsigned int haptic_right = (data & 0x0F) << 4;
      unsigned int haptic_left  =  data & 0xF0;

      haptic_right |= haptic_right >> 4;
      haptic_left  |= haptic_left  >> 4;

      haptic_left  = (haptic_left  * motorStrength) / 255;
      haptic_right = (haptic_right * motorStrength) / 255;

      analogWrite(MOTOR_LEFT, haptic_left);
      analogWrite(MOTOR_RIGHT, haptic_right);
    }
  } else {
    stopMotors();
  }

  // ── Keepalive ohne Akku ─────────────────
  if (connected && millis() - lastKeepAlive >= KEEPALIVE_MS) {
    lastKeepAlive = millis();

    bleuart.write((uint8_t)100);
    Serial.println("[BLE] Keepalive sent: 100");
  }

  // ── Pairing LED ─────────────────────────
  if (pairingMode) {
    unsigned long now = millis();

    if (now - pairingStart >= PAIRING_DURATION_MS) {
      stopPairingMode();
    } else if (now - lastPairingBlink >= PAIRING_BLINK_MS) {
      lastPairingBlink = now;
      pairingLedState = !pairingLedState;
      digitalWrite(LED_PIN, pairingLedState);
    }
  }

  // ── LED blinkt wenn nicht verbunden ─────
  if (!connected && !pairingMode) {
    unsigned long now = millis();

    if (now - lastIdleBlink >= IDLE_BLINK_MS) {
      lastIdleBlink = now;
      digitalWrite(LED_PIN, HIGH);
      delay(80);
      digitalWrite(LED_PIN, LOW);
    }
  }

  // ── Button ──────────────────────────────
  bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;

  if (buttonPressed) {
    if (buttonWasReleased) {
      buttonPressStart = millis();
      buttonWasReleased = false;
      sleepTriggered = false;

      Serial.println("[BUTTON] Pressed");
    }

    unsigned long heldTime = millis() - buttonPressStart;

    if (!sleepTriggered && heldTime >= SLEEP_HOLD_MS) {
      sleepTriggered = true;
      Serial.println("[BUTTON] 5s held -> Sleep");
      goToSleep();
    }

  } else {
    if (!buttonWasReleased) {
      unsigned long heldTime = millis() - buttonPressStart;

      if (!sleepTriggered && heldTime >= PAIRING_HOLD_MS) {
        Serial.println("[BUTTON] Released after 3s -> Pairing");

        if (pairingMode) {
          stopPairingMode();
        } else {
          startPairingMode();
        }
      }

      Serial.println("[BUTTON] Released");
    }

    buttonWasReleased = true;
    sleepTriggered = false;
  }
}