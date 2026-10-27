# DARKLOTUS - Combat System & Controls Guide

## 1. Input Layout (Tekken 4-Button Standard)

DarkLotus uses the iconic Tekken 4-button fighting scheme:

```
          [2: Right Punch] (Triangle / Y / I)
[1: Left Punch] (Square / X / U)          [4: Right Kick] (Circle / B / K)
          [3: Left Kick] (X / A / J)
```

---

## 2. Keyboard & Gamepad Default Bindings

### Player 1

| Action | Keyboard Key | Gamepad (Xbox/PlayStation) |
|---|---|---|
| **Sidestep Up (Z-)** | `W` | D-Pad Up / Left Stick Up |
| **Sidestep Down (Z+) / Crouch** | `S` | D-Pad Down / Left Stick Down |
| **Retreat / Guard** | `A` | D-Pad Left / Left Stick Left |
| **Advance / Forward Dash** | `D` | D-Pad Right / Left Stick Right |
| **1: Left Punch (LP)** | `U` | `X` / `Square` |
| **2: Right Punch (RP)** | `I` | `Y` / `Triangle` |
| **3: Left Kick (LK)** | `J` | `A` / `Cross` |
| **4: Right Kick (RK)** | `K` | `B` / `Circle` |
| **1 + 2 (Dual Punch)** | `O` | Right Bumper (`RB` / `R1`) |
| **3 + 4 (Dual Kick)** | `L` | Right Trigger (`RT` / `R2`) |
| **Rage Art / Heat Burst** | `SPACE` | Left Bumper (`LB` / `L1`) |
| **Pause Menu** | `ESCAPE` | `Start` / `Options` |

### Player 2

| Action | Keyboard Key | Gamepad 1 |
|---|---|---|
| **Sidestep Up (Z-)** | `Up Arrow` | D-Pad Up |
| **Sidestep Down (Z+) / Crouch** | `Down Arrow` | D-Pad Down |
| **Movement** | `Left` / `Right Arrow` | D-Pad Left / Right |
| **1: Left Punch (LP)** | `Numpad 4` | `X` |
| **2: Right Punch (RP)** | `Numpad 5` | `Y` |
| **3: Left Kick (LK)** | `Numpad 1` | `A` |
| **4: Right Kick (RK)** | `Numpad 2` | `B` |
| **1 + 2** | `Numpad 6` | `RB` |
| **3 + 4** | `Numpad 3` | `RT` |
| **Rage Art / Heat Burst** | `Numpad Enter` | `LB` |

---

## 3. Core Move List & Frame Data

### Universal Tekken Moves

| Command | Move Name | Hit Level | Startup | On Block | On Hit | Properties |
|---|---|---|---|---|---|---|
| `1` (LP) | **Flash Jab** | HIGH | i10 | +1 | +8 | Fastest interrupt move; +1 advantage on block |
| `2` (RP) | **Straight Cross** | MID | i12 | -3 | +6 | Fast mid check; stops crouching opponents |
| `3` (LK) | **Torso Kick** | MID | i14 | -5 | +7 | Safe mid attack |
| `4` (RK) | **High Roundhouse** | HIGH | i16 | -8 | +14 (Launch) | Launches on Counter-Hit for full juggle! |
| `Down + 2` (Down + RP) | **Wind God Uppercut** | MID | i15 | -10 | Launch | **Primary Launcher**: Propels opponent into air for juggles! |
| `Down + 3` (Down + LK) | **Dragon Low Sweep** | LOW | i16 | -13 | Knockdown | Hits low under standing guards; ducks under highs |
| `SPACE` (Rage Art) | **Lotus Crush** | MID | i20 | -18 | Huge Damage | Activated when HP <= 25%; deals up to 3200 damage! |

---

## 4. Tekken Defense & Guard Rules

1. **Standing Guard (`Back`)**:
   - Blocks **HIGH** and **MID** attacks.
   - Vulnerable to **LOW** sweeps and throws.
2. **Crouching Guard (`Down-Back` / `Down`)**:
   - Ducks entirely under **HIGH** moves (causing them to whiff).
   - Blocks **LOW** attacks.
   - Vulnerable to **MID** attacks (e.g. launchers and hopkicks).
3. **Sidestepping (`W` or `S`)**:
   - Moves along the 3D Z-axis perpendicular to the opponent.
   - Evades linear straight attacks, opening up side-turn and back-turn counter-attacks!
