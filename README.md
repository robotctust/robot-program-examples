# robot-program-examples

為機器人研究社課程設計的 Arduino 機器人範例程式集。

---

## 範例總覽

| 資料夾                                                     | 功能                            |
| --------------------------------------------------------- | ------------------------------- |
| [`line-follower-basic`](./line-follower-basic/)           | 基礎循線（直線或曲線）              |
| [`line-follower-advanced`](./line-follower-advanced/)     | 進階循線（地圖陣列 + 路口判斷）     | 
| [`bluetooth-remote-control`](./bluetooth-remote-control/) | 藍牙遙控                         |
| [`bluetooth-remote-control-em`](./bluetooth-remote-control-em/) | 藍牙遙控（編碼器 PID 穩速，**不同車體**，見下方說明） |
| [`line-follower-advanced-em`](./line-follower-advanced-em/) | 進階循線（編碼器 PID 穩速，**不同車體**，見下方說明） |

---

## 硬體需求

- **Arduino Uno**（或相容板）
- **IR 紅外線感測器 × 5**（底部循線用）
- **L9110S 馬達驅動模組**
- **直流馬達 × 2**
- **藍牙模組**（僅 `bluetooth-remote-control` 需要）
- **18650 電池 × 2 + 雙槽電池盒** （或其他電池合適電源）

> **注意：`-em` 結尾的範例（`bluetooth-remote-control-em`、`line-follower-advanced-em`）是為另一台機器車設計的**，硬體與腳位和其他範例不同，不能直接互換：
>
> - 馬達驅動改用 **L293D**，並使用 **帶編碼器的直流馬達**（CPR 624）做 PID 穩速
> - 開發板為 **Arduino Uno R4**；藍牙走硬體 `Serial1`（D0 RX / D1 TX，9600 baud），舊款 Uno R3 沒有 `Serial1`，無法編譯
>
> | 功能 | 腳位 |
> | --- | --- |
> | 右輪 ENA / IN1 / IN2 | 5 / 6 / 7 |
> | 左輪 ENB / IN3 / IN4 | 10 / 8 / 9 |
> | 左輪編碼器 A / B | 2 / 4 |
> | 右輪編碼器 A / B | 3 / 11 |
> | 五路循線感測器（左 → 右） | A0 / A1 / A2 / A3 / A4 |

---

## 使用方式

1. 用 Arduino IDE 開啟對應資料夾內的 `.ino` 檔案。
2. 依據你的車體調整速度與馬達校正參數。
3. 選擇正確的板子與序列埠，上傳程式。
