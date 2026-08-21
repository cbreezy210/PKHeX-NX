# 🗺️ PKHeX-NX — Public Roadmap
A native Nintendo Switch homebrew save editor for Pokémon (Gen 8 & 9) — no PC required.

---

## ✅ Shipped — v0.9.0 (The Core Engine & Scarlet Foundation)
*The math, the cryptography, and the safety nets. Forensically verified against hardware.*

- [x] **SwishCrypto Engine:** SHA256 footer, 127-byte xorpad, SCBlock/SCXorShift32 port.
- [x] **Trainer Mapping:** ID, Name, Money editing with 4-step safety-gated NAND commit.
- [x] **Box Viewer:** 32x30 grid navigation with verified 344-byte stride.
- [x] **PK9 Codec:** Decrypt8/Encrypt8, LCG + 4x80 block shuffle, checksum healing.
- [x] **Stat Editor:** IVs, EVs, Nature, Tera Type, Force Shiny.
- [x] **Box Toolkit:** Inject, Clone, Delete, and Export to `.pk9`.
- [x] **Dual-File Commit:** Writes to both `main` and `backup` simultaneously to prevent stale-file fallbacks.
- [x] **Forensic Verification:** 0 diffs between main/backup, stride mathematically proven.

## 🔧 Ready to Ship — v0.9.1 (The Final Hardware Gate)
- [ ] **3-Photo Verification Cycle:** Final hardware proof of the clone/delete/commit loop to close Phase 1.

## 🟡 Coming Next — v1.0.0 (The Multi-Game & Content Expansion)
*Target Release: Late 2026*

### 🌍 Multi-Game Framework
- [ ] **Boot-time Game Selector:** Probes Title IDs, lists installed saves, manual override.
- [ ] **Violet Support:** Reuses PK9 stack (TID `0x01008F6008C5E000`).
- [ ] **Legends: Z-A Support:** PK9 codec reuse with Z-A specific block keys.
- [ ] **Sword/Shield, BDSP, Legends: Arceus:** PK8, PB8, and PA8 codec ports.

### 📖 Data & Content
- [ ] **SD Name Tables:** `species.txt`, `items.txt`, `moves.txt`, `abilities.txt` parsed into vectors at boot.
- [ ] **Native Keyboard Search:** Press (+) to open swkbd and instantly search the databases by name.
- [ ] **Bag Editor:** Parse and edit the item block with a visual slot picker.
- [ ] **Legalize Button:** Adjusts OT/TID/HT/memories for Home/online play safety.
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
- [ ] **Crash Logging:** Write a `crash.log` to the SD card on unrecoverable errors.

### 🎒 Quality of Life
- [ ] **Box Loadouts (Y):** Save/load named box snapshots (team sets, living-dex starters) to SD.
- [ ] **Favorites (X):** One-button `.pk9` preset injects (6IV Ditto, competitive starters).
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
- [ ] **Multi-Language UI:** Localized `.txt` tables and UI strings (EN, ES, FR, DE, JA, KO).
- [ ] **Modular Plugin System:** Community devs can write JSON scripts for new offsets.
- [ ] **Wi-Fi Cloud Backup:** Upload backups off-console for true cloud saves.

---
*Built with devkitPro/libnx. Educational project — always keep backups!*
