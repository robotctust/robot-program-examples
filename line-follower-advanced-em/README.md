# line-follower-advanced-em

[`line-follower-advanced`](../line-follower-advanced/) 的 **Uno R4 編碼器車版本**。七階查表循線、地圖驅動路口、預觸發、感測閂鎖、虛線容忍等邏輯完全相同，運作方式與 `MAP[]` 寫法請參考該資料夾的 README。

---

## 檔案說明

| 檔案 | 說明 |
|---|---|
| `line-follower-advanced-em.ino` | 主程式 — 感測、七階循線、地圖路口決策 |
| `motor.ino` | 馬達穩速模組（與 `bluetooth-remote-control-em` 相同）— 編碼器 + PID |

---

## 與 `line-follower-advanced` 的差異

- **開發板**：Arduino Uno R4
- **感測器腳位**：最左 → 最右為 `A0` / `A1` / `A2` / `A3` / `A4`
- **馬達**：L293D + 編碼器馬達，腳位見 `motor.ino`
- **速度單位改為 RPM**（此馬達最高約 50 RPM），由 PID 穩速，因此移除了 `RIGHT_MOTOR_PCT` / `LEFT_MOTOR_PCT` 平衡校正
- **不能用 `delay()`**：PID 需要 `motorTick()` 持續被呼叫，轉彎時改用 `waitMs()` / `waitMidBlack()`

---

## 可調整參數

```cpp
const double SPEED_FORWARD = 30;   // 直走速度（RPM）
const double SPEED_CORRECT = 30;   // 極偏時的旋轉速度
const double SPEED_TURN    = 30;   // 路口轉彎速度
const double SPEED_GENTLE  = 20;   // 七階循線輕微修正的慢側速度
const double SPEED_STRONG  = 10;   // 七階循線強力修正的慢側速度（反轉）

const unsigned long FORWARD_PUSH_MS = 200;  // 轉彎前前推時間
const unsigned long TURN_45_MS  = 125;      // 45° 盲轉時間
const unsigned long TURN_90_MS  = 275;      // 90° 盲轉時間
const unsigned long TURN_135_MS = 450;      // 135° 盲轉時間
```

> 速度與盲轉時間是依原版比例換算的起始值，尚未在實車上調校，請先量 90° 再按比例推算 45° / 135°。
