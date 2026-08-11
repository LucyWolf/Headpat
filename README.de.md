[🇬🇧 English](README.md) | 🇩🇪 Deutsch

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
