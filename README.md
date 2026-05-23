# robot-program-examples

Arduino robot example programs for Robotics Club curriculum.

> 繁體中文版請見 [README.zh-TW.md](./README.zh-TW.md)

---

## Examples

| Folder                                                    | Description                                                           |
| --------------------------------------------------------- | --------------------------------------------------------------------- |
| [`line-follower-basic`](./line-follower-basic/)           | Basic line following                                                  |
| [`line-follower-advanced`](./line-follower-advanced/)     | Advanced line following with path map array and intersection handling |
| [`bluetooth-remote-control`](./bluetooth-remote-control/) | Bluetooth remote control via HC-05/HC-06                              |

---

## Hardware Requirements

- **Arduino Uno** (or compatible board)
- **IR sensors × 5** (bottom-mounted, for line detection)
- **L9110S motor driver module**
- **DC motors × 2**
- **Bluetooth module** (only for `bluetooth-remote-control`)
- **18650 battery × 2 + dual-cell battery holder** (or other suitable power source)

---

## Getting Started

1. Open the `.ino` file inside the target folder with Arduino IDE.
2. Tune the speed and motor balance constants to match your hardware.
3. Select the correct board and port, then upload.