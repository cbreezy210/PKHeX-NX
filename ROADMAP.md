# 🗺️ PKHeX-NX — Public Roadmap
A native Nintendo Switch homebrew save editor for Pokémon (Gen 8 & 9) — no PC required.

### 🎯 Roadmap Scope: What This IS / IS NOT

| This Roadmap IS... | This Roadmap IS NOT... |
| :--- | :--- |
| A technical blueprint for native, offline save editing. | A promise of online/competitive cheating tools. |
| Focused on cryptography, safety, and legality. | A commitment to bypassing Nintendo's anti-cheat or ban systems. |
| Driven by community feedback and hardware validation. | A strict timeline with guaranteed, fixed release dates. |

---

### 🏗️ Project Architecture & Phase Tree

```text
PKHeX-NX Development Phases
├── Phase 1: Core Engine & Safety (v0.9.x) ✅
│   ├── SwishCrypto (SCBlock / Xorpad)
│   ├── PK9 Codec (Gen 9 SV)
│   └── Military-Grade Safety Net
├── Phase 2: Content & Multi-Game (v1.0.x) 🟡
│   ├── Gen 8 & Z-A Codecs
│   ├── Bag Editor & Items
│   └── Pokédex & Legality Automation
├── Phase 3: UI Overhaul (v1.1.x) 🟠
│   ├── SDL2 Graphical Renderer
│   └── Touch & Highlight Navigation
└── Phase 4: Deep Edits & Community (v2.0.x) 🔴
    ├── Cross-Game Bank
    └── Localization & Plugins
```

---

### 📂 Installation & SD Card Layout
*(Reference for upcoming features that interact with the SD card)*

```text
sdmc:/ (Root of your SD Card)
├── switch/
│   └── PKHeX-NX/
│       └── PKHeX-NX.nro          <-- The Homebrew App
├── pkhex-nx-assets/               <-- REQUIRED Data Files
│   ├── species.txt, stats.txt, abilities.txt
│   ├── growth.txt, learnsets.bin
│   ├── pp.txt, moves.txt
│   └── items.txt (Coming in v1.0)
└── pkhex-nx-backups/              <-- AUTO-CREATED Safety Net
    ├── SC_main_YYYYMMDD_HHMMSS.bak
    └── VI_main_YYYYMMDD_HHMMSS.bak
```

---

## ✅ Shipped — v0.9.0 (The Core Engine & Scarlet Foundation)
The math, the cryptography, and the safety nets. Forensically verified against hardware.

- [x] **SwishCrypto Engine:** SHA256 footer, 127-byte xorpad, SCBlock/SCXorShift32 port.
- [x] **Trainer Mapping:** ID, Name, Money editing with 4-step safety-gated NAND commit.
- [x] **Box Viewer:** 32x30 grid navigation with verified 344-byte stride.
- [x] **PK9 Codec:** Decrypt8/Encrypt8, LCG + 4x80 block shuffle, checksum healing.
- [x] **Stat Editor:** IVs, EVs, Nature, Tera Type, Force Shiny.
- [x] **Box Toolkit:** Inject, Clone, Delete, and Export to .pk9.
- [x] **Dual-File Commit:** Writes to both main and backup simultaneously to prevent stale-file fallbacks.
- [x] **Forensic Verification:** 0 diffs between main/backup, stride mathematically proven.

## ✅ Shipped — v0.9.1 (Hardware Validation)
End-to-end on-console verification with photographic proof. Phase 1 closed.

- [x] **Final Hardware Validation Pass:** Full on-console verification of the clone/delete/commit loop.
- [x] **Native-Format Box Delete:** Delete writes the exact empty-slot encoding the game itself uses, for perfectly clean round trips.
- [x] **Box Storage Verification:** Slot layout and encryption verified against the game's own autosaves for 100% round-trip fidelity.

## ✅ Shipped — v0.9.2 (Multi-Game Framework & Database Generator)
The editor grows beyond a single save. Phase 2 core closed.

- [x] **Boot-Time Game Selector:** Auto-detects installed Scarlet & Violet saves with manual override.
- [x] **Violet Support:** Full PK9 stack validated on a second title.
- [x] **Database Generator:** Mint any species on-console with computed stats, chosen ball, moves, and trainer metadata — renders cleanly in-game.
- [x] **26-Ball Picker:** Complete ball roster with correct in-game ball IDs.
- [x] **Native Keyboard Search:** swkbd species search across all 960 box slots.
- [x] **Base-Stat Engine:** Real per-species base stats parsed from SD assets for accurate stat computation.

## ✅ Shipped — v0.9.3 (Full Legality Generator + Manual Move Picker)
Every generated Pokémon is now 100% legal, level-accurate, and fully customizable.

- [x] **Public Beta release:** Drag-and-drop .zip + Applet Mode guard.
- [x] **Real Abilities:** Each species gets one of its legal abilities (Ability 1, Ability 2, or Hidden) with correct ID and bitflag.
- [x] **Manual Legal Move Picker (M11):** Press [Y] in the generator to hand-pick up to 4 moves from the species' full legal learnset at or below the chosen level. Displays real move names parsed from moves.txt.
- [x] **Correct Max PP:** Per-move PP values sourced from the game's move data tables.
- [x] **Level Picker:** Choose any level from 1 to 100 for the generated Pokémon.
- [x] **Egg Generation:** Toggle to create a valid egg (level 1, egg flag set, correct hatch cycles, level-1 moveset).
- [x] **Growth-Correct EXP:** All 6 experience curves (Med Fast / Erratic / Fluctuating / Med Slow / Fast / Slow) implemented per-species — no more level drift.
- [x] **Embedded Homebrew Icon:** Custom Poké Ball + hexagon + circuit logo displayed in the Homebrew Menu.

## 🟢 Coming Soon — v0.9.4 (Stability & Hardening Sprint)
Bulletproofing the core architecture for the v1.0 transition.

- [ ] **Paldean Native Species Fix:** Correctly map internal Gen 9 species IDs to National Dex for Pokémon #917+ (Tarountula and beyond) to ensure accurate names and sprites in the Box Viewer.
- [ ] **SD Card Space Validation:** Check available free space (requires 2x save size) before creating backups to prevent silent write failures.
- [ ] **Round-Trip Crypto Verification:** Mathematically verify the parser on boot (`encrypt(decrypt(x)) == x`) to guarantee data integrity before any editing begins.

## 🟡 Coming Next — v1.0.0 (The Multi-Game & Content Expansion)
*Target Release: Late 2026*

### 🧬 Legality & Quality of Life
- [ ] **Pokédex Auto-Registration:** Injected Pokémon are automatically registered as seen/caught in the in-game Pokédex.
- [ ] **Handling Trainer Updates:** Pokémon moved between saves get HT fields updated exactly like an in-game trade.
- [ ] **Advanced Box Search:** Filter boxes by shiny, egg, level range, and perfect IVs.
- [ ] **7-Star Raid Capture Reset:** Clear the "already captured" flag on Unrivaled raids for re-capturing.
- [ ] **Sandwich Recipe Injector:** One-click bag fill with exact ingredients for shiny/egg power sandwiches.

### 🌍 Multi-Game Framework
- [ ] **Legends: Z-A Support:** PK9 codec reuse with Z-A specific block keys.
- [ ] **Sword/Shield, BDSP, Legends: Arceus:** PK8, PB8, and PA8 codec ports.

### 📖 Data & Content
- [ ] **SD Name Tables:** items.txt parsed into vectors at boot for Bag Editor.
- [ ] **Bag Editor:** Parse and edit the item block with a visual slot picker.
- [ ] **Advanced Legality Validation:** Legalize button that aligns OT/TID/HT/memories and runs deep legality checks so edited Pokémon stay safe for HOME and online play.
- [ ] **Sanity Checks & Safe Mode:** Prevents impossible values, restricts edits to offline-safe limits.

## 🟠 Mid-Term — v1.1.0 (The "ACNH Treatment" UI Overhaul)
*Target Release: Q1 2027*

### 🎨 UI & Graphics
- [ ] **Full Graphical UI:** SDL2 renderer replaces the text console (themed header, logo, crisp TTF text).
- [ ] **Highlight-Bar Navigation:** Clean, centered layout to prevent text overflow.
- [ ] **Graceful App Exit:** Dedicated "Quit App" option in the main menu.

### 🛡️ Safety & Reliability
- [ ] **"Dry Run" Preview Mode:** See exactly what will change before writing to NAND.
- [ ] **Rolling Backup Manager:** Keep the last 3 backups with timestamps and a visual picker.
- [ ] **Game Version Validator:** Warn if save offsets don't match the current game patch.
- [ ] **Crash Logging:** Write a crash.log to the SD card on unrecoverable errors.

### 🎒 Quality of Life & Automation
- [ ] **"Living Dex" Smart Automator:** Scans the Pokédex for missing entries and generates a wishlist — or auto-injects legal .pk9 templates — to complete your dex.
- [ ] **Box Loadouts (Y):** Save/load named box snapshots (team sets, living-dex starters) to SD.
- [ ] **Favorites (X):** One-button .pk9 preset injects (6IV Ditto, competitive starters).
- [ ] **Batch Operations:** Max IVs, set nature, or legalize an entire box at once.

## 🔴 Long-Term — v2.0.0 (Deep Edits & Community Overhaul)
*Target Release: 2027+*

### 🏆 Completion Cheats
- [ ] **Pokédex Completion:** Mark all seen/caught flags for the regional and national dex.
- [ ] **Story & Raid Flags:** Unlock legendary cutscenes, gym badges, and manipulate raid seeds.

### 🎨 UI & Graphics
- [ ] **Sprite Atlas:** Blit tiny Pokémon icons next to each box slot.
- [ ] **Save File "Diff" Viewer:** Compare backup and current save side-by-side before writing.

### 🌍 Community & Localization
- [ ] **Multi-Language UI:** Localized .txt tables and UI strings (EN, ES, FR, DE, JA, KO).
- [ ] **Modular Plugin System:** Community devs can write JSON scripts for new offsets.
- [ ] **Wi-Fi Cloud Backup:** Upload backups off-console for true cloud saves.

---

### 🎮 Target Compatibility Matrix

| Game | Status | Save Format | Boxes | Slots/Box | Target Version |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Pokémon Scarlet** | ✅ Implemented | SCBlock (`main`) | 32 | 30 | v0.9.0+ |
| **Pokémon Violet** | ✅ Implemented | SCBlock (`main`) | 32 | 30 | v0.9.2+ |
| **Legends: Z-A** | 🟡 Planned | SCBlock (`main`) | 32 | 30 | v1.0.0 |
| **Sword/Shield** | 🟡 Planned | SCBlock (`main`) | 32 | 30 | v1.0.0 |
| **Brilliant Diamond/Shining Pearl** | 🟡 Planned | Flat binary | 40 | 30 | v1.0.0 |
| **Legends: Arceus** | 🟡 Planned | SCBlock (`main`) | 32 | 30 | v1.0.0 |

---

### ⚠️ Known Gaps (Strictly Aligned with Roadmap)

*These limitations are known and explicitly scheduled for resolution to prevent feature drift.*

* **Text-Only UI:** The current interface is a developer-focused text console. 
  * *Fix scheduled:* **v1.1.0** (SDL2 Graphical UI).
* **No Bag/Item Editing:** You cannot currently edit your inventory, TMs, or key items. 
  * *Fix scheduled:* **v1.0.0** (Bag Editor).
* **No Pokédex Registration:** Injected or generated Pokémon do not automatically register as "caught" in your in-game Pokédex. 
  * *Fix scheduled:* **v1.0.0** (Pokédex Auto-Registration).
* **Gen 9 Internal ID Quirk:** Pokémon introduced in Paldea (#917 Tarountula and higher) may display incorrect internal index numbers in the raw data view. 
  * *Fix scheduled:* **v0.9.4** (Paldean Native Species Fix).

---

### 🛡️ Backup & Safety System Philosophy

Safety is the core pillar of this roadmap. Every feature that touches NAND is gated by our safety net:

* **File Paths:** `sdmc:/pkhex-nx-backups/SC_main_YYYYMMDD_HHMMSS.bak` (or `VI_` for Violet).
* **Byte Verification:** Backups are mathematically verified after creation to ensure data integrity.
* **Dual-File Commit:** When committing changes to NAND, the app writes to both the main save file and the backup save file simultaneously.
* **One-Button Rollback:** Press `[R]` on the main menu to instantly restore from the newest timestamped backup.

> 💡 **The Honest Note:** No save editor is 100% bulletproof. While PKHeX-NX uses military-grade verification and dual-file commits, SD cards can fail, power can cut out, and edge cases in Gen 9's complex save structure can still exist. *Always* keep a secondary backup on your PC using a tool like JKSV or Checkpoint before doing major edits. Our safety net is your first line of defense, not your only one.

---

### 🤝 Credits & Acknowledgments

* **kwsch & The PKHeX Team:** For the foundational research, cryptography documentation, and the legendary desktop editor that inspired this project. ([GitHub](https://github.com/kwsch/PKHeX))
* **Switchbrew & devkitPro:** For maintaining the libnx toolchain that makes native Switch homebrew possible. ([Website](https://devkitpro.org))
* **The GBATemp & r/HomebrewSwitch Communities:** For early testing, hardware validation, and relentless feedback during the beta phase.
* **Insektaure (pkHouse, pkBakery):** For setting the gold standard in native Switch Pokémon UI/UX and safety patterns.
* **GameBrew:** For hosting the [Install Guide & Documentation Mirror](https://www.gamebrew.org/wiki/PKHeX-NX).

---

### ⚖️ Responsible Use & Disclaimer

* **Offline Use Only:** Do not use edited Pokémon in online multiplayer, official tournaments, or ranked battles. 
* **Respect the Ecosystem:** Do not distribute edited saves or Pokémon as your own work.
* **Educational Purpose:** This software is provided "as-is" for research into cryptography and data structures. Modifying save data always carries a risk. I am not responsible for corrupted saves, lost Pokémon, or banned consoles. Use at your own risk! 
* **Not Affiliated:** This project is not affiliated with, endorsed by, or sponsored by Nintendo, Game Freak, or The Pokémon Company.

---
Built with devkitPro/libnx. Educational project — always keep backups!