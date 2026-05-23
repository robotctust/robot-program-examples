# line-follower-advanced

> 繁體中文版請見 [README.zh-TW.md](./README.zh-TW.md)

Extends basic line following with a map-driven intersection system and a 7-level line correction table. The route is defined as a `MAP[]` array of `MapStep` structs; the robot follows the line between intersections and executes the next map entry each time the sensor pattern matches.

Ideal for competition tracks with a fixed route.

---

## How It Works

The robot operates in two layers simultaneously:

**1. 7-level line following**

Instead of a simple left/mid/right check, the robot reads all five sensors and selects one of seven correction modes:

| Sensor state | Correction |
|---|---|
| Only Mid on | Straight |
| Left + Mid on | Gentle left |
| Mid + Right on | Gentle right |
| Left only on | Strong left |
| Right only on | Strong right |
| Far-left only on | Spin left |
| Far-right only on | Spin right |

On dashed-line segments where all sensors go dark, the robot holds the last correction mode for up to `DASH_GAP_CYCLES` iterations before falling back to straight.

**2. Map-driven intersection handling**

Each entry in `MAP[]` contains:
- `prePattern` — sensor pattern that must be seen *first* (optional, set to `0` to skip)
- `pattern` — main trigger pattern
- `action` — what to do when triggered (`ACT_LEFT_90`, `ACT_RIGHT_45`, `ACT_END`, …)

The robot checks the current sensor reading against `MAP[juncIdx]` each loop. When the pattern matches and the cooldown has elapsed, the action fires and `juncIdx` advances. A pre-trigger step (`juncPreSeen`) disambiguates junctions that share the same main pattern.

Turns use a fixed blind-spin time (`TURN_*_MS`) followed by a sensor-guided search for the centre line.

---

## Customising the Route

Edit the `MAP[]` array to match your track. Each row is one intersection in order:

```cpp
const MapStep MAP[] = {
  { 0,       PAT_RIGHT3, ACT_RIGHT_90 },  // 1. right turn
  { 0,       PAT_LEFT3,  ACT_LEFT_90  },  // 2. left turn
  { PAT_L_M, PAT_ML_M,   ACT_LEFT_135 },  // 3. wide left (pre-trigger required)
  { 0,       PAT_ALL,    ACT_END      },  // 4. stop at finish
};
```

### Trigger Patterns (`pattern` / `prePattern`)

Sensor layout, left to right: **FAR_LEFT · LEFT · MID · RIGHT · FAR_RIGHT**

| Pattern | Sensors on | Typical junction |
|---|---|---|
| `PAT_LEFT3` | FAR_LEFT + LEFT + MID | Left T-junction or left branch |
| `PAT_RIGHT3` | MID + RIGHT + FAR_RIGHT | Right T-junction or right branch |
| `PAT_ML_M` | LEFT + MID | Narrow-angle left branch (main trigger) |
| `PAT_L_M` | FAR_LEFT + MID | Narrow-angle left branch (pre-trigger) |
| `PAT_DOT_R` | MID + FAR_RIGHT | Dashed-line right path |
| `PAT_R_M` | RIGHT + FAR_RIGHT | Narrow-angle right branch (pre-trigger) |
| `PAT_CTR` | LEFT + MID + RIGHT | Centre zone (outer two sensors dark) |
| `PAT_ALL` | All five | Crossroad or finish line |

Set `prePattern` to `0` if no pre-trigger is needed. Use a non-zero `prePattern` when two consecutive junctions share the same main pattern — the robot must see `prePattern` first before `pattern` can fire.

### Actions

| Action | Behaviour |
|---|---|
| `ACT_LEFT_45` | Blind-spin left for `TURN_45_MS`, then search for centre line |
| `ACT_LEFT_90` | Push forward, blind-spin left for `TURN_90_MS`, then search |
| `ACT_LEFT_135` | Push forward, blind-spin left for `TURN_135_MS`, then search |
| `ACT_RIGHT_45` | Blind-spin right for `TURN_45_MS`, then search |
| `ACT_RIGHT_90` | Push forward, blind-spin right for `TURN_90_MS`, then search |
| `ACT_RIGHT_135` | Push forward, blind-spin right for `TURN_135_MS`, then search |
| `ACT_END` | Stop immediately and set `robot_done = true` |

---

## Adjustable Parameters

```cpp
const int SPEED_FORWARD = 110;  // Straight cruising speed (0–255)
const int SPEED_CORRECT =  80;  // Spin speed for extreme deviation
const int SPEED_TURN    = 100;  // Intersection turn speed
const int SPEED_GENTLE  =  55;  // Slow-side speed for gentle correction
const int SPEED_STRONG  =  25;  // Slow-side speed for strong correction (slight reverse)

const unsigned long TURN_45_MS  = 125;  // Blind-spin duration for 45° turn
const unsigned long TURN_90_MS  = 275;  // Blind-spin duration for 90° turn
const unsigned long TURN_135_MS = 450;  // Blind-spin duration for 135° turn
const unsigned long JUNC_CD_MS  = 500;  // Junction cooldown — prevents re-triggering
const int           DASH_GAP_CYCLES = 20; // Blank-segment tolerance before going straight
```

---

## Notes

- Set `SENSOR_BLACK = LOW` if your IR sensors output LOW on black.
- The robot starts after a 1-second delay. Replace `delay(1000)` in `setup()` with a button trigger for competition starts.
- Blind-spin durations (`TURN_*_MS`) must be tuned to your vehicle's speed and battery level. Measure 90° first, then scale linearly.
