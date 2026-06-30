# HeatPett

A wearable haptic headpat device for VRChat. When someone pats your avatar's head, two vibration motors on the physical device respond in real time.

## How it works

```
VRChat  →  OSC  →  HeatPett Server  →  USB Serial  →  Dongle  →  BLE  →  HeatPett
```

1. VRChat sends OSC contact data when the headpat area is touched
2. The **HeatPett Server** (Windows app) receives the OSC signal and converts it to motor commands
3. Commands are sent via USB serial to the **Dongle** (nice!nano plugged into PC)
4. The Dongle forwards them over BLE to the **HeatPett** (nice!nano worn on head)
5. The HeatPett drives two ERM vibration motors — left and right independently

## Hardware

- 2× nice!nano (nRF52840) — one worn, one as USB dongle
- 2× ERM vibration motors
- LiPo battery
- Tactile button
- LED

*Schaltplan folgt.*

## Firmware

This repository contains the firmware for the **worn device** (HeatPett).

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

### Serial commands

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

- [dongel_NRF](https://github.com/LucyWolf/dongel_NRF) — Dongle firmware
- HeatPett Server — Windows app (OSC receiver + BLE bridge)
