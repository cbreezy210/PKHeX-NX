# 🎮 PKHeX-NX (Native Switch Pokémon Save Editor)

A 100% native Nintendo Switch homebrew application for editing Pokémon save files (Gen 9: Scarlet & Violet) directly on your console. No PC, no pulling the SD card, no complicated dumping tools required.

> ⚠️ **STATUS: ACTIVE DEVELOPMENT (v0.9.3)**  
> The core engine, cryptography, safety features, and legality-aware generator are fully built, forensically verified, and hardware-validated on-console. The graphical UI is currently in development. Watch this repo for updates!

## ✨ Core Features (v0.9.3)

*   **Native Switch Execution:** Edits decrypted save data directly in RAM and writes back to NAND securely.
*   **Multi-Game Framework:** Boot-time selector auto-detects installed Scarlet & Violet saves.
*   **Military-Grade Safety Net:**
    *   Automatic byte-verified SD card backups before every single write.
    *   "Egg-Killer" dual-file commit protocol (writes to both `main` and `backup` to prevent stale fallbacks).
    *   One-button emergency rollback to the newest backup.
*   **SwishCrypto Engine:** Fully ported SHA256 footer verification, 127-byte xorpad decryption, and SCBlock/SCXorShift32 parsing.
*   **PK9 Codec:** Native decryption, block-shuffling, and re-encryption for Pokémon Scarlet/Violet.
*   **Legality-Aware Database Generator:**
    *   Mint any species with real abilities (Ability 1 / 2 / Hidden).
    *   **Manual Legal Move Picker:** Hand-pick up to 4 moves from the species' legal learnset at or below the chosen level (real move names, no hex IDs!).
    *   Correct max PP per move.
    *   Growth-correct EXP across all 6 experience curves.
    *   Optional egg generation (valid egg flag, hatch cycles, level-1 moveset).
    *   26-ball picker with correct in-game ball IDs.
*   **Comprehensive Box Toolkit:**
    *   **Inject:** Load `.pk9` files directly from your SD card.
    *   **Clone:** Duplicate any Pokémon to another box slot.
    *   **Delete:** Permanently remove a Pokémon (writes game-empty encoding).
    *   **Export:** Save any box Pokémon to a `.pk9` file on your SD card.
*   **Stat & Trait Editor:** Modify IVs, EVs, Natures, Tera Types, and Force Shiny statuses with automatic checksum healing.
*   **Hardware-Validated Round Trips:** Every operation verified end-to-end against the game's autosave format — clean renders, zero ghost artifacts.

## 🗺️ Roadmap
See [ROADMAP.md](ROADMAP.md) for the full list of shipped engine features and planned multi-game/UI updates.

## 🎮 Controls (Current Text UI)

| Button | Action |
| :--- | :--- |
| **D-Pad** | Navigate boxes, slots, and menus |
| **A** | Open Pokémon / Select / Confirm |
| **B** | Back / Cancel |
| **X** | Clone selected Pokémon |
| **Y** | Delete selected Pokémon / **Pick Moves (in Generator)** |
| **ZL** | Inject from `.pk9` (box viewer) / Emergency rollback (main menu) |
| **ZR** | Export to `.pk9` |
| **L / R** | Change Box |
| **+** | Search species / Change game |

## 🛠️ Building from Source
Requires `devkitPro` with `switch-dev` installed.

```bash
make
```

## 🧪 Generating SD Assets (Developers)
The generator reads per-species data tables from the SD card at `sdmc:/pkhex-nx-assets/`. To build them, run the bundled script against a local PKHeX source checkout:

```bash
py tools/make_assets.py <path-to-pkhex-ref> <output-dir>
```

This produces `stats.txt`, `abilities.txt`, `growth.txt`, `learnsets.bin`, `pp.txt`, and `moves.txt`. Copy those (plus your `species.txt` name table) into `sdmc:/pkhex-nx-assets/` on the SD card.

## ⚠️ Disclaimer
This is an educational project. Modifying save data always carries a risk. While PKHeX-NX features a robust backup and safety-net system, always keep your own manual backups. I am not responsible for corrupted saves, lost Pokémon, or banned consoles. Use at your own risk!

Built with devkitPro/libnx. Educational project — always keep backups!
