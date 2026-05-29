# bluetooth-remote-control

透過手機以藍牙傳送指令來控制兩輪機器人。機器人透過藍牙模組接收單字元指令，並驅動馬達執行對應動作。

---

## 檔案說明

| 檔案 | 說明 |
|---|---|
| `bluetooth-remote-control.ino` | 主程式 — 藍牙接收迴圈與指令分派 |
| `motor.ino` | 馬達控制抽象層 — 底層 PWM 控制與高層動作函式 |

---

## 指令對應

| 字元 | 動作 |
|---|---|
| `a` | 前進 |
| `d` | 後退 |
| `c` | 左轉 |
| `b` | 右轉 |
| `s` | 停止 |
| _（其他任何字元）_ | 停止 |

---

## 推薦手機 App

使用本社自行以 App Inventor 2 開發的 Android 遙控 App，專為此機器人設計：

**[robot-bt-remote-app](https://github.com/robotctust/robot-bt-remote-app)** — 至 [Releases](https://github.com/robotctust/robot-bt-remote-app/releases) 頁面下載 APK 安裝檔。

---

## 可調整參數

```cpp
int speed = 100; // 馬達速度（0–255）
```
