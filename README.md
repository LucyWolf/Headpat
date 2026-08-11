**Language / Sprache:** [🇬🇧 English](#headpat) | [🇩🇪 Deutsch](#headpat-1)

---

# Headpat

A wearable haptic headpat device for VRChat. When someone pats your avatar's head, two vibration motors on the physical device respond in real time.

## How it works

```
VRChat  →  OSC  →  Headpat Server  →  BLE  →  Headpat
```

1. VRChat sends OSC contact data when the headpat area is touched
2. The **Headpat Server** (Windows/Linux app) receives the OSC signal and converts it to motor commands
3. Commands are sent directly via Bluetooth to the **Headpat** (nice!nano worn on head)
4. The Headpat drives two ERM vibration motors — left and right independently

## Hardware

- 1× nice!nano (nRF52840)
- 2× ERM vibration motors
- LiPo battery
- Tactile button
- LED

## Firmware

This repository contains the firmware for the worn device.

| Pin | Function |
|-----|----------|
| D14 (P1.11) | Left motor PWM |
| D16 (P0.10) | Right motor PWM |
| P0.15 | Status LED |
| D1 (P0.06) | Button |

### Button behavior

| Action | Effect |
|--------|--------|
| Hold 3s, release | Toggle pairing mode |
| Hold 5s | Sleep (press button to wake) |

### LED behavior

| State | Pattern |
|-------|---------|
| Not connected | Short blink every 2s |
| Pairing mode | Fast blink (150ms) |
| Button held | On |
| Connected | Off |

### BLE protocol (NUS)

The device exposes a Nordic UART Service (NUS). The Headpat Server communicates via single-byte commands:

| Byte | Description |
|------|-------------|
| `0x00`–`0xEF` | Motor command — high nibble = left (0–15), low nibble = right (0–15) |
| `0xFA` | Sleep |
| `0xFB` | Request firmware version |
| `0xFC` | Request battery level |
| `0xFD` | Unpair |
| `0xFE` | Pair |

Responses are sent as plain text over NUS TX, prefixed with `[BAT]` or `[VER]`.

### USB debug commands

Connect via USB at 115200 baud:

| Command | Description |
|---------|-------------|
| `info` | Show status (version, battery, uptime) |
| `battery` | Raw battery voltage and percentage |
| `pairing` | Toggle pairing mode |
| `s=<0-255>` | Set motor strength |
| `reboot` | Restart device |
| `dfu` | Enter UF2 bootloader |
| `meow` | :3 |

## Building

Requires [PlatformIO](https://platformio.org/).

```bash
pio run
```

The firmware builds automatically on every push via GitHub Actions. Download the latest `headpat-firmware.uf2` from [Releases](../../releases).

## Flashing

1. Double-tap the reset button on the nice!nano — it mounts as a USB drive
2. Copy `headpat-firmware.uf2` onto the drive
3. The device reboots automatically

## Related

- [Headpat Server](https://github.com/LucyWolf/Headpat-Server) — Windows/Linux app (OSC receiver + BLE bridge)

---

# Headpat

Ein tragbares haptisches Headpat-Gerät für VRChat. Wenn jemand den Kopf deines Avatars streichelt, reagieren zwei Vibrationsmotoren am physischen Gerät in Echtzeit.

## So funktioniert es

```
VRChat  →  OSC  →  Headpat Server  →  BLE  →  Headpat
```

1. VRChat sendet OSC-Kontaktdaten wenn die Headpat-Zone berührt wird
2. Der **Headpat Server** (Windows/Linux-App) empfängt das Signal und wandelt es in Motorbefehle um
3. Befehle werden direkt per Bluetooth an den **Headpat** (nice!nano am Kopf) gesendet
4. Der Headpat steuert zwei ERM-Vibrationsmotoren — links und rechts unabhängig

## Hardware

- 1× nice!nano (nRF52840)
- 2× ERM-Vibrationsmotoren
- LiPo-Akku
- Taster
- LED

## Firmware

Dieses Repository enthält die Firmware für das getragene Gerät.

| Pin | Funktion |
|-----|----------|
| D14 (P1.11) | Linker Motor PWM |
| D16 (P0.10) | Rechter Motor PWM |
| P0.15 | Status-LED |
| D1 (P0.06) | Taster |

### Taster-Verhalten

| Aktion | Effekt |
|--------|--------|
| 3s halten, loslassen | Pairing-Modus umschalten |
| 5s halten | Schlafen (Knopf drücken zum Aufwecken) |

### LED-Verhalten

| Zustand | Muster |
|---------|--------|
| Nicht verbunden | Kurzes Blinken alle 2s |
| Pairing-Modus | Schnelles Blinken (150ms) |
| Knopf gehalten | An |
| Verbunden | Aus |

### BLE-Protokoll (NUS)

Das Gerät stellt einen Nordic UART Service (NUS) bereit. Der Headpat Server kommuniziert über einzelne Bytes:

| Byte | Beschreibung |
|------|--------------|
| `0x00`–`0xEF` | Motorbefehl — High-Nibble = links (0–15), Low-Nibble = rechts (0–15) |
| `0xFA` | Schlafen |
| `0xFB` | Firmware-Version anfragen |
| `0xFC` | Akkustand anfragen |
| `0xFD` | Entkoppeln |
| `0xFE` | Koppeln |

Antworten werden als Klartext über NUS TX gesendet, mit `[BAT]` oder `[VER]` als Präfix.

### USB-Debug-Befehle

Per USB bei 115200 Baud verbinden:

| Befehl | Beschreibung |
|--------|--------------|
| `info` | Status anzeigen (Version, Akku, Laufzeit) |
| `battery` | Rohe Akkuspannung und Prozent |
| `pairing` | Pairing-Modus umschalten |
| `s=<0-255>` | Motorstärke setzen |
| `reboot` | Gerät neu starten |
| `dfu` | UF2-Bootloader starten |
| `meow` | :3 |

## Bauen

Benötigt [PlatformIO](https://platformio.org/).

```bash
pio run
```

Die Firmware wird bei jedem Push automatisch via GitHub Actions gebaut. Die neueste `headpat-firmware.uf2` unter [Releases](../../releases) herunterladen.

## Flashen

1. Reset-Button auf dem nice!nano doppelt drücken — er wird als USB-Laufwerk eingehängt
2. `headpat-firmware.uf2` auf das Laufwerk kopieren
3. Das Gerät startet automatisch neu

## Verwandte Projekte

- [Headpat Server](https://github.com/LucyWolf/Headpat-Server) — Windows/Linux-App (OSC-Empfänger + BLE-Bridge)
