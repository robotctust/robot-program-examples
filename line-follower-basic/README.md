# line-follower-basic

> 繁體中文版請見 [README.zh-TW.md](./README.zh-TW.md)

Basic line following using three sensors (Left, Mid, Right). The robot steers based on which sensor detects the line. The two outer sensor pins are declared and reserved for future expansion.

---

## How It Works

The robot reads the three center sensors every loop:

| Mid | Left | Right | Action |
|---|---|---|---|
| ✅ | — | — | Forward |
| ❌ | ✅ | — | Spin left |
| ❌ | ❌ | ✅ | Spin right |
| ❌ | ❌ | ❌ | Stop |

The outer sensors (`FAR_LEFT`, `FAR_RIGHT`) are also read and will trigger a spin if the inner ones miss the line entirely.

---

## Adjustable Parameters

```cpp
const int SPEED_FORWARD = 100;   // Forward speed (0–255)
const int SPEED_TURN    = 100;   // Turn speed (0–255)
const int RIGHT_MOTOR_PCT = 100; // Right motor balance (%)
const int LEFT_MOTOR_PCT  = 100; // Left motor balance (%)
```

Adjust `RIGHT_MOTOR_PCT` / `LEFT_MOTOR_PCT` if the robot drifts to one side on a straight line.

---

## Notes

- IR sensor logic (`0`/`1` for line detection) may vary by manufacturer. Verify with `Serial.println()` before running.
- This example uses `digitalRead()`. If your sensors support analog output, switching to `analogRead()` with a threshold can improve tolerance on worn or faded tracks.
