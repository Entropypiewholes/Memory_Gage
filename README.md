# Digimon Memory

Flipper Zero tool to track the shared **Memory Gauge** in the official Digimon Card Game (TCG).

- Starts at 0 (standard setup).
- Range: -10 (you have 10 memory) to +10 (opponent has 10 memory).
- Clean visual gauge + big numbers for quick glances during play.

## Controls (on device)
- **UP (top button)**: Move gauge toward opponent side (+1). Typical "pay cost" action on your turn.
- **DOWN (bottom button)**: Move gauge toward your side (-1). Gain memory from effects or opponent paying.
- **OK (center, short)**: Reset to 0.
- **LEFT (short)**: Pass your turn — instantly sets to +3 on opponent's side (per official rules).
- **BACK**: Exit the app.

Hold UP or DOWN for rapid continuous adjustment (via repeat events).

The gauge is **absolute** (YOU left, OPP right). Pressing UP always slides the marker right on screen.

## Rules Notes (for reference)
- Memory counter starts at 0.
- Pay a card's cost by moving the counter toward opponent's side by that many spaces.
- If the counter is at 1+ on opponent's side after costs/effects resolve, your turn ends.
- Max 10 memory per side. Effects that would exceed 10 are lost.
- Pass: memory is set to exactly 3 on opponent's side (regardless of prior position).
- This app is a tracker only — you are responsible for applying the rules (costs, effects, turn ends).

See the official Digimon Card Game rule manual for full details.

## Build (Windows / PowerShell)
1. Install ufbt (one time):
   ```
   pip install ufbt
   ```
2. In this directory:
   ```
   ufbt
   ```
   or for rapid dev loop:
   ```
   ufbt launch   # builds + installs + runs on connected Flipper (USB)
   ```

First build will download the toolchain/SDK (can take a few minutes).

## Install Manually
- Copy the produced `.fap` (e.g. from `dist/digimon_memory.fap` or `build/digimon_memory.fap` after `ufbt`) to your Flipper's SD card at:
  `/ext/apps/Tools/digimon_memory.fap`
- On the Flipper: Apps → Tools → Digimon Memory

## Verification Tips
- Launch → should show 0, centered marker, "0 memory".
- UP x4 from 0 → +4, "Opponent has 4 memory", marker 4 ticks right.
- DOWN past 0 → negative values show "You have N memory".
- Values clamp at +/- 10.
- Restart app → last value is restored from SD (apps_data/digimon_memory/memory.txt).

## Future / Polish Ideas (not in v1)
- Settings for sound / quick presets.
- Long-press for +/-5 jumps.
- Nicer themed icon + assets.

This is a community custom app, not affiliated with Bandai, Toei, or the official Digimon Card Game.

Enjoy your games!
