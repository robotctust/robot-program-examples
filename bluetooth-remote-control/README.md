# bluetooth-remote-control

透過手機以藍牙傳送指令來控制兩輪機器人。機器人透過藍牙模組接收單字元指令，並驅動馬達執行對應動作。

---

## 檔案說明

| 檔案 | 說明 |
|---|---|
| `bluetooth-remote-control.ino` | 主程式 — 藍牙接收迴圈與指令分派 |
| `motor.ino` | 馬達控制抽象層 — 底層 PWM 控制與高層動作函式 |

---

## 接線

| BT06 腳位 | Arduino 腳位 |
|---|---|
| TXD | D7（`BT_RX`） |
| RXD | D2（`BT_TX`） |

藍牙鮑率：9600。

---

## 指令對應

### 方向指令

| 字元 | 動作 |
|---|---|
| `F` | 前進 |
| `B` | 後退 |
| `L` | 原地左轉 |
| `R` | 原地右轉 |
| `S` | 停止 |

### 速度指令

| 字元 | 速度 |
|---|---|
| `0` | 0（停止） |
| `1`–`9` | 線性對應 100–200 |

- 大小寫皆可（小寫會自動轉成大寫）。
- 程式會記住目前的方向，調整速度會立即套用到目前動作，不必重送方向指令。
- 速度設為 `0` 後需再送 `1`–`9` 才會恢復移動。
- 其他字元會被忽略，機器人維持原本動作。

---

## 推薦手機 App

使用本社自行以 App Inventor 2 開發的 Android 遙控 App，專為此機器人設計：

**[robot-bt-remote-app](https://github.com/robotctust/robot-bt-remote-app)** — 至 [Releases](https://github.com/robotctust/robot-bt-remote-app/releases) 頁面下載 APK 安裝檔。

---

## 可調整參數

```cpp
const int BT_RX = 7;  // 接 BT06 的 TXD
const int BT_TX = 2;  // 接 BT06 的 RXD
int speedVal = 100;   // 開機預設速度（100–200）
```

左右輪速度不平衡時，可調整 `motor.ino` 中的 `LEFT_MOTOR_PCT` / `RIGHT_MOTOR_PCT`。
