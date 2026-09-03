# 🎮 PKHeX-NX (Native Switch Pokémon Save Editor)
**The Pokémon SV companion tool that never leaves your Switch.**

A 100% native Nintendo Switch homebrew application for editing Pokémon save files (Gen 9: Scarlet & Violet) directly on your console. No PC, no pulling the SD card, no complicated dumping tools required.

[![Version](https://img.shields.io/github/v/release/cbreezy210/PKHeX-NX)](https://github.com/cbreezy210/PKHeX-NX/releases) [![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

> **Note:** This is an active public beta (v0.9.3) and an educational project. Always keep backups of your save files. Modifying save data always carries a risk. The author is not responsible for corrupted saves, lost Pokémon, or banned consoles. Use offline and at your own risk. Not affiliated with Nintendo, Game Freak, or The Pokémon Company.
>
> **Responsible Use:** This app is built for offline, single-player enjoyment and research. Please do not use edited Pokémon in online multiplayer, official tournaments, or ranked battles, and do not distribute edited saves or Pokémon as your own work.

## 🎯 What This App Is
| PKHeX-NX IS: | PKHeX-NX IS NOT: |
|---|---|
| A safe, offline save editor with automatic byte-verified backups | A replacement for Pokémon HOME or a cloud-sync tool |
| 100% on-console (no PC, phone, or sysmodules required) | A tool for cheating in online competitive multiplayer |
| An educational reverse-engineering & cryptography project | A "magic button" that guarantees zero risk to your save data |

## ⚠️ CRITICAL LAUNCH INSTRUCTIONS
For full memory and SD card access, you **MUST** launch this app via **Title Override** (hold [R] while launching a game like Pokémon Scarlet or Violet).

**Do NOT launch it from the Album / Applet mode** — the app will refuse to run and show you exactly how to relaunch correctly, preventing save-access crashes!

## ✨ Features (v0.9.3)
* **Native Switch Execution:** Edits decrypted save data directly in RAM and writes back to NAND securely.
* **Multi-Game Framework:** Boot-time selector auto-detects installed Scarlet & Violet saves.
* **Military-Grade Safety Net:** Automatic byte-verified SD card backups before every single write, a dual-file commit protocol (writes to both main and backup to prevent stale fallbacks), and one-button emergency rollback.
* **SwishCrypto Engine:** Fully ported SHA256 footer verification, 127-byte xorpad decryption, and SCBlock/SCXorShift32 parsing.
* **PK9 Codec:** Native decryption, block-shuffling, and re-encryption for Pokémon Scarlet/Violet.
* **Legality-Aware Database Generator:** Mint any species with real abilities (Ability 1 / 2 / Hidden), correct max PP, growth-correct EXP across all 6 experience curves, optional egg generation, and a 26-ball picker with correct in-game ball IDs.
* **Manual Legal Move Picker:** Press [Y] in the generator to hand-pick up to 4 moves from the species' full legal learnset at or below the chosen level (real move names, no hex IDs!).
* **Comprehensive Box Toolkit:** Inject `.pk9` files, Clone, Delete (native empty-slot encoding), and Export to `.pk9` — all from the 32x30 box viewer.
* **Stat & Trait Editor:** Modify IVs, EVs, Natures, Tera Types, and Force Shiny statuses with automatic checksum healing.
* **Native Keyboard Search:** swkbd species search across all 960 box slots.
* **Applet Mode Guard:** The app hard-blocks Album/Applet launches and shows on-screen instructions for relaunching correctly.
* **Embedded Homebrew Icon:** Custom Poké Ball + hexagon + circuit logo displayed in the Homebrew Menu.
* **Hardware-Validated Round Trips:** Every operation verified end-to-end against the game's autosave format — clean renders, zero ghost artifacts.

## 🛡️ Backup System & Safety
When loading a game save on Switch, a full backup copy is created **before** any modification:

```text
sdmc:/pkhex-nx-backups/SC_main_YYYYMMDD_HHMMSS.bak   (Scarlet)
sdmc:/pkhex-nx-backups/VI_main_YYYYMMDD_HHMMSS.bak   (Violet)
```

**One-Button Rollback:** Press [ZL] on the main menu to instantly restore from your newest timestamped backup.

**Safety Guarantees:**
* Every save operation creates a byte-verified backup *before* writing to NAND* Dual-file commit writes to both the main and backup save simultaneously
* SHA256 footers and checksums are re-verified and healed on every edit
* The app refuses to run in Applet Mode to prevent save-access crashes

> 💡 **The Honest Note:** No save editor is 100% bulletproof. While PKHeX-NX uses military-grade verification and dual-file commits, SD cards can fail, power can cut out, and edge cases in Gen 9's complex save structure can still exist. *Always* keep a secondary backup on your PC using a tool like JKSV or Checkpoint before doing major edits. Our safety net is your first line of defense, not your only one.

## ✅ Compatibility
| Game | Status | Save Format | Boxes | Slots/Box | Tested FW |
|---|---|---|---|---|---|
| **Pokémon Scarlet** | ✅ Implemented | SCBlock (`main`) | 32 | 30 | 22.5.0 |
| **Pokémon Violet** | ✅ Implemented | SCBlock (`main`) | 32 | 30 | 22.5.0 |
| **Legends: Z-A** | 🟡 Planned | SCBlock (`main`) | 32 | 30 | - |
| **Sword/Shield** | 🟡 Planned | SCBlock (`main`) | 32 | 30 | - |
| **BDSP** | 🟡 Planned | Flat binary | 40 | 30 | - |
| **Legends: Arceus** | 🟡 Planned | SCBlock (`main`) | 32 | 30 | - |

Tested by the author on FW 22.5.0 (Atmosphère 1.11.2 E). When Nintendo ships a Pokémon update, compatibility is re-verified and noted in the changelog.

## 🎨 Coming in v0.9.4: Stability & Hardening Sprint
Based on your feedback! The Paldean Native Species Fix (#917+), SD Card Space Validation, and Round-Trip Crypto Verification are next up. Then v1.0 brings the big content expansion — still deciding between prioritizing the **Bag Editor** or **Pokédex Auto-Registration** first — which would you rather see? Let us know on GitHub Discussions!

## 📥 Installation
Ensure your Switch is running Custom Firmware (Atmosphère).

### Standard Installation (SD Card)

1. Download the latest `PKHeX-NX-Beta-v0.9.3.zip` from the [Releases](https://github.com/cbreezy210/PKHeX-NX/releases) page.
2. Extract the contents directly to the **root** of your SD card. This creates the following structure:

```text
📦 SD (Root)
┣━ 📂 switch
┃  ┗━ 📂 PKHeX-NX
┃     ┗━ 📜 PKHeX-NX.nro
┣━ 📂 pkhex-nx-assets
┃  ┣━ 📄 species.txt
┃  ┣━ 📄 stats.txt
┃  ┣━ 📄 abilities.txt
┃  ┣━ 📄 growth.txt
┃  ┣━ 📄 learnsets.bin
┃  ┣━ 📄 pp.txt
┃  ┗━ 📄 moves.txt
┗━ 📂 pkhex-nx-backups (auto-created)
   ┣━ 📄 SC_main_YYYYMMDD_HHMMSS.bak
   ┗━ 📄 VI_main_YYYYMMDD_HHMMSS.bak
```

3. Fully close Pokémon Scarlet/Violet (do not leave it suspended).
4. Hold [R] and launch Pokémon Scarlet or Violet from the Home Menu to open the Homebrew Menu.
5. Select "PKHeX-NX" to launch (do **not** run the game at the same time!).
### Alternative Installation (DBI / Homebrew App Store)

For users with DBI installed on their Switch, or once approved on the Homebrew App Store:

1. Download `PKHeX-NX-DBI-v0.9.3.zip` from the [Releases](https://github.com/cbreezy210/PKHeX-NX/releases) page.
2. Install the `.nro` via DBI (MTP responder or network), or grab it from the Homebrew App Store.
3. Ensure the `pkhex-nx-assets/` folder is on your SD card root (see structure above).
4. Launch via Title Override (hold [R] on Pokémon Scarlet/Violet from the Home Menu).

## 🎮 Controls
| Button | Action |
|---|---|
| **D-Pad** | Navigate boxes, slots, and menus |
| **A** | Open Pokémon / Select / Confirm |
| **B** | Back / Cancel |
| **X** | Clone selected Pokémon |
| **Y** | Delete selected Pokémon / Pick Moves (in Generator) |
| **ZL** | Inject from .pk9 (box viewer) / Emergency rollback (main menu) |
| **ZR** | Export to .pk9 |
| **L / R** | Change Box |
| **+** | Search species / Change game |

## 🗺️ Roadmap & Known Gaps
See [ROADMAP.md](ROADMAP.md) for the full list of shipped and planned features.

**Known Gaps (v0.9.3):**
* Text-only developer console UI; full SDL2 graphical UI planned for v1.1
* No Bag/Item editing yet (planned for v1.0)
* No Pokédex registration yet (planned for v1.0)
* Gen 9 Internal ID quirk for #917+ (fix queued for v0.9.4)

## 🛠️ Building from Source
Requires devkitPro with switch-dev (libnx) installed.

```bash
git clone https://github.com/cbreezy210/PKHeX-NX.git
cd PKHeX-NX
make
```

This produces `PKHeX-NX.nro`. The build is fully self-contained — **no game data dumps or extra files are required to compile.**

> ℹ️ **Note for developers:** All species / move / stat data is loaded **at runtime** from `sdmc:/pkhex-nx-assets/` (see the "Generating SD Assets" section below). The `romfs/*.dat` embedding rules in the Makefile are legacy leftovers from an earlier design and are **not** part of the build — you can safely ignore them.

### 🧪 Generating SD Assets (Developers)
The generator reads per-species data tables from the SD card at `sdmc:/pkhex-nx-assets/`. To build them, run the bundled script against a local PKHeX source checkout:

`py tools/make_assets.py <path-to-pkhex-ref> <output-dir>`

This produces `stats.txt`, `abilities.txt`, `growth.txt`, `learnsets.bin`, `pp.txt`, and `moves.txt`. Copy those (plus your `species.txt` name table) into `sdmc:/pkhex-nx-assets/` on the SD card.

---

## 🙏 Credits
* **kwsch and the PKHeX project** — foundational save-structure research, cryptography documentation, and the legendary desktop editor: https://github.com/kwsch/PKHeX
* **devkitPro and libnx communities** — Switch homebrew tooling: https://devkitpro.org
* **The GBATemp & r/HomebrewSwitch communities** — for early testing, hardware validation, and relentless feedback
* **Insektaure (pkHouse, pkBakery)** — for setting the gold standard in native Switch Pokémon UI/UX and safety patterns
* **GameBrew** — for hosting the Install Guide & Documentation Mirror

## ⚖️ Disclaimer
This is an educational project. Always keep backups of your save files. Modifying save data always carries a risk. I am not responsible for corrupted saves, lost Pokémon, or banned consoles. Use at your own risk!

## 💬 Community & External Links
* **GitHub Releases (Download):** https://github.com/cbreezy210/PKHeX-NX/releases
* **GitHub Discussions (Feedback & Feature Votes):** https://github.com/cbreezy210/PKHeX-NX/discussions
* **GameBrew Wiki (Install Guide & Docs Mirror):** https://www.gamebrew.org/wiki/PKHeX-NX
* **GBATemp Devlog Thread:** https://gbatemp.net/threads/pkhex-nx-native-switch-pokemon-save-editor-scarlet-violet-devlog-sneak-peek.683964/
* **Reddit Release Thread:** https://www.reddit.com/r/homebrew/s/Foe8Q8cJsN
