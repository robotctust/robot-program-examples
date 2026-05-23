# robot-program-examples

為機器人研究社課程設計的 Arduino 機器人範例程式集。

> English version: [README.md](./README.md)

---

## 範例總覽

| 資料夾                                                     | 功能                            |
| --------------------------------------------------------- | ------------------------------- |
| [`line-follower-basic`](./line-follower-basic/)           | 基礎循線（直線或曲線）              |
| [`line-follower-advanced`](./line-follower-advanced/)     | 進階循線（地圖陣列 + 路口判斷）     | 
| [`bluetooth-remote-control`](./bluetooth-remote-control/) | 藍牙遙控                         |

---

## 硬體需求

- **Arduino Uno**（或相容板）
- **IR 紅外線感測器 × 5**（底部循線用）
- **L9110S 馬達驅動模組**
- **直流馬達 × 2**
- **藍牙模組**（僅 `bluetooth-remote-control` 需要）
- **18650 電池 × 2 + 雙槽電池盒** （或其他電池合適電源）

---

## 使用方式

1. 用 Arduino IDE 開啟對應資料夾內的 `.ino` 檔案。
2. 依據你的車體調整速度與馬達校正參數。
3. 選擇正確的板子與序列埠，上傳程式。
