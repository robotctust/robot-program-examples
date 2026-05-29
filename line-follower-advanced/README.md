# line-follower-advanced

在基礎循線之上增加地圖驅動的路口系統與七階循線修正表。路線以 `MAP[]` 陣列的 `MapStep` 結構體定義；機器人在路段之間持續循線，每次感測器模式符合地圖中的下一個路口時，執行對應動作並推進到下一步。

適合應用於指定路線的循線競賽場地。

---

## 運作邏輯

機器人同時執行兩個層次：

**1. 七階查表式循線**

不再只判斷左/中/右三種狀態，而是讀取全部五顆感測器，從七種修正模式中選一：

| 感測器狀態 | 修正方式 |
|---|---|
| 只有中間亮 | 直走 |
| 左 + 中亮 | 輕微左修 |
| 中 + 右亮 | 輕微右修 |
| 只有左亮 | 強力左修 |
| 只有右亮 | 強力右修 |
| 只有最左亮 | 原地左旋 |
| 只有最右亮 | 原地右旋 |

在虛線段所有感測器熄滅時，機器人會沿用上次的修正模式最多 `DASH_GAP_CYCLES` 次，才切換為直走。

**2. 地圖驅動的路口判斷**

`MAP[]` 的每一筆包含：
- `prePattern` — 必須先看到的感測器模式（設為 `0` 則略過此步驟）
- `pattern` — 主觸發模式
- `action` — 觸發後執行的動作（`ACT_LEFT_90`、`ACT_RIGHT_45`、`ACT_END` 等）

每個 loop 都會將當前感測器讀值與 `MAP[juncIdx]` 比對。模式命中且冷卻時間已過，動作觸發並 `juncIdx++`。預觸發步驟（`juncPreSeen`）可消歧義主模式相同但動作不同的路口。

轉彎方式：先盲轉固定時間（`TURN_*_MS`），再靠感測器尋回中線。

---

## 更換路線

修改 `MAP[]` 陣列以配合你的賽道，每列代表一個路口（依序排列）：

```cpp
const MapStep MAP[] = {
  { 0,       PAT_RIGHT3, ACT_RIGHT_90 },  // 1. 右轉
  { 0,       PAT_LEFT3,  ACT_LEFT_90  },  // 2. 左轉
  { PAT_L_M, PAT_ML_M,   ACT_LEFT_135 },  // 3. 大左彎（需預觸發）
  { 0,       PAT_ALL,    ACT_END      },  // 4. 終點停止
};
```

### 觸發模式（`pattern` / `prePattern`）

感測器由左至右排列：**FAR_LEFT · LEFT · MID · RIGHT · FAR_RIGHT**

| 模式 | 亮起的感測器 | 對應路口情境 |
|---|---|---|
| `PAT_LEFT3` | FAR_LEFT + LEFT + MID | 左 T 字路口或左側分支 |
| `PAT_RIGHT3` | MID + RIGHT + FAR_RIGHT | 右 T 字路口或右側分支 |
| `PAT_ML_M` | LEFT + MID | 窄角左路主觸發 |
| `PAT_L_M` | FAR_LEFT + MID | 窄角左路預觸發 |
| `PAT_DOT_R` | MID + FAR_RIGHT | 虛線右路 |
| `PAT_R_M` | RIGHT + FAR_RIGHT | 窄角右路預觸發 |
| `PAT_CTR` | LEFT + MID + RIGHT | 中央區（兩側外感測器未亮） |
| `PAT_ALL` | 全部五顆 | 十字路口或終點 |

`prePattern` 設為 `0` 代表不需要預觸發，直接等待 `pattern`。若相鄰兩個路口的主模式相同，可用非零的 `prePattern` 消歧義：機器人必須先看到 `prePattern`，`pattern` 才能觸發。

### 動作

| 動作 | 行為 |
|---|---|
| `ACT_LEFT_45` | 盲轉左 `TURN_45_MS`，再靠感測器找中線 |
| `ACT_LEFT_90` | 前推後盲轉左 `TURN_90_MS`，再靠感測器找中線 |
| `ACT_LEFT_135` | 前推後盲轉左 `TURN_135_MS`，再靠感測器找中線 |
| `ACT_RIGHT_45` | 盲轉右 `TURN_45_MS`，再靠感測器找中線 |
| `ACT_RIGHT_90` | 前推後盲轉右 `TURN_90_MS`，再靠感測器找中線 |
| `ACT_RIGHT_135` | 前推後盲轉右 `TURN_135_MS`，再靠感測器找中線 |
| `ACT_END` | 立即停止並設定 `robot_done = true` |

---

## 可調整參數

```cpp
const int SPEED_FORWARD = 110;  // 直走速度（0–255）
const int SPEED_CORRECT =  80;  // 極偏時的旋轉速度
const int SPEED_TURN    = 100;  // 路口轉彎速度
const int SPEED_GENTLE  =  55;  // 七階循線輕微修正的慢側速度
const int SPEED_STRONG  =  25;  // 七階循線強力修正的慢側速度（微反轉）

const unsigned long TURN_45_MS  = 125;  // 45° 盲轉時間
const unsigned long TURN_90_MS  = 275;  // 90° 盲轉時間
const unsigned long TURN_135_MS = 450;  // 135° 盲轉時間
const unsigned long JUNC_CD_MS  = 500;  // 路口冷卻，防止連續誤觸發
const int           DASH_GAP_CYCLES = 20; // 虛線空白容忍次數
```

---

## 注意事項

- 若 IR 感測器偵測到黑線時輸出 LOW，請將 `SENSOR_BLACK` 改為 `LOW`。
- 機器人啟動後等待 1 秒再出發，可將 `setup()` 中的 `delay(1000)` 替換為按鈕觸發。
- 盲轉時間（`TURN_*_MS`）需依車速與電池電量手動微調，建議先測量 90° 後再按比例推算 45° / 135°。
