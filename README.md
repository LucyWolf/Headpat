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

*Schaltplan folgt.*

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
