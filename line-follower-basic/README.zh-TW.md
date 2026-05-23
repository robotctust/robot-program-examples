# line-follower-basic

> English version: [README.md](./README.md)

以左、中、右三顆感測器實作基礎循線邏輯。機器人根據感測到黑線的位置決定前進、左轉或右轉。最左與最右感測器的腳位已宣告保留，供後續擴充。

---

## 運作邏輯

每個迴圈讀取三顆中央感測器的狀態：

| 中 | 左 | 右 | 動作 |
|---|---|---|---|
| ✅ | — | — | 前進 |
| ❌ | ✅ | — | 左轉 |
| ❌ | ❌ | ✅ | 右轉 |
| ❌ | ❌ | ❌ | 停止 |

最外側的兩顆感測器（`FAR_LEFT`、`FAR_RIGHT`）也會讀取，若內側三顆都沒偵測到線時作為補救轉向。

---

## 可調整參數

```cpp
const int SPEED_FORWARD = 100;   // 直行速度（0–255）
const int SPEED_TURN    = 100;   // 轉向速度（0–255）
const int RIGHT_MOTOR_PCT = 100; // 右馬達校正（%）
const int LEFT_MOTOR_PCT  = 100; // 左馬達校正（%）
```

若機器人在直線段固定偏向一側，調整 `RIGHT_MOTOR_PCT` / `LEFT_MOTOR_PCT` 來補償。

---

## 注意事項

- IR 感測器的黑線邏輯（`0`/`1`）可能因廠牌不同而相反，上機前請先用 `Serial.println()` 確認，若數值相反，將程式中所有判斷條件的 `0` 和 `1` 互換即可。
- 此範例使用 `digitalRead()`。若感測器支援類比輸出，改用 `analogRead()` 搭配閾值判斷可以提升在磨損或褪色賽道上的容錯率。
