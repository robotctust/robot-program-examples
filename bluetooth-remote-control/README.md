# bluetooth-remote-control

> 繁體中文版請見 [README.zh-TW.md](./README.zh-TW.md)

Controls a two-wheeled robot via Bluetooth commands sent from a smartphone. The robot receives single-character commands over a serial Bluetooth connection and drives the motors accordingly.

---

## Files

| File | Description |
|---|---|
| `bluetooth-remote-control.ino` | Main sketch — Bluetooth receive loop and command dispatch |
| `motor.ino` | Motor driver abstraction — low-level PWM control and high-level movement functions |

---

## Command Mapping

| Character | Action |
|---|---|
| `a` | Forward |
| `d` | Backward |
| `c` | Spin left |
| `b` | Spin right |
| `s` | Stop |
| _(any other)_ | Stop |

---

## Recommended App

Use the Android app developed by our club with App Inventor 2, designed specifically for this robot:

**[robot-bt-remote-app](https://github.com/robotctust/robot-bt-remote-app)** — download the APK from the [Releases](https://github.com/robotctust/robot-bt-remote-app/releases) page.

---

## Adjustable Parameters

```cpp
int speed = 100; // Motor speed (0–255)
```
