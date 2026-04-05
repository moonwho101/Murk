<p align="center">
  <img src="artwork/murk256.jpg" alt="Murk — 1998 isometric dungeon crawler" width="640">
</p>

<h1 align="center">Murk</h1>

<p align="center">
  <strong>A 1998 isometric dungeon crawler — open-source, buildable, and playable on modern Windows</strong>
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="MIT License"></a>
  <img src="https://img.shields.io/badge/platform-Windows-0078d4" alt="Windows">
  <img src="https://img.shields.io/badge/language-C%2B%2B-00599C" alt="C++">
  <img src="https://img.shields.io/badge/era-1998-orange" alt="1998">
  <img src="https://img.shields.io/badge/build-Visual%20Studio%202022-purple" alt="Visual Studio 2022">
  <img src="https://img.shields.io/badge/DirectX-DirectDraw%20%7C%20DirectSound-brightgreen" alt="DirectX">
</p>

<p align="center">
  <a href="#-play-now">Play Now</a> · <a href="#-screenshots">Screenshots</a> · <a href="#-build-from-source">Build</a> · <a href="#-technical-deep-dive">Technical</a> · <a href="#-contributing">Contributing</a>
</p>

---

Murk is a fast, arcade-style fantasy dungeon crawler created by **Mark Longo** in 1998, now fully open-sourced under the MIT license. Choose from four characters, descend into procedurally generated dungeons, fight monsters, collect treasure, and survive.

Originally built for **Windows 95** using the Win32 API, MFC, DirectDraw, DirectSound, and DirectPlay, this repository preserves the complete source code — updated to compile cleanly in **Visual Studio 2022** — along with a ready-to-run binary.

> *"To dungeons deep and caverns old / We must away, ere break of day, / To find our long-forgotten gold."* — J.R.R. Tolkien

## 🎮 Play Now

**No build required.** A pre-built executable is included:

1. Download or clone this repository.
2. Open the `bin/` folder.
3. Run **`Murk.exe`**.

Works on **Windows 10** and **Windows 11**.

## 📸 Screenshots

<img src="artwork/Murk01.jpg" alt="Murk gameplay — isometric dungeon exploration" width="800">

<img src="artwork/dungeon.jpg" alt="Murk dungeon level with monsters and torches" width="800">

<img src="artwork/multiplayer.jpg" alt="Murk multiplayer promotional screenshot" width="800">

## ✨ Features

- **Isometric 2D dungeon crawling** — explore multi-level dungeons filled with monsters, traps, and loot
- **Four playable characters** — each with distinct play styles
- **Procedural dungeon generation** — different layout every time
- **Mouse, keyboard, and joystick support** — flexible input for any setup
- **Save and load** — continue your adventure anytime
- **MIDI soundtrack** — atmospheric period-authentic music
- **Legacy multiplayer code** — DirectPlay networking preserved in source (TCP/IP, IPX/SPX, modem, serial)
- **Compact codebase** — ~30 source files, easy to read and learn from

## 📊 Quick Facts

| | |
| :--- | :--- |
| **Genre** | Arcade-style isometric dungeon crawler |
| **Original release** | 1998–1999 |
| **Language** | C++ (~30 source files) |
| **Graphics** | DirectDraw (2D isometric) |
| **Audio** | DirectSound + MIDI |
| **Networking** | DirectPlay (TCP/IP, IPX/SPX, modem, serial) |
| **Platform** | Win32 |
| **Build tool** | Visual Studio 2022 |
| **License** | MIT |

## 🕹️ Controls

### Mouse

| Action | Input |
| :--- | :--- |
| Move | Left click |
| Fire | Right click |
| Stairs / ladders | Double left click |

### Keyboard

| Action | Key |
| :--- | :--- |
| Move | Arrow keys |
| Fire | Ctrl |
| Stairs / ladders | Shift |
| Ring of Protection | R |
| Drop treasure | T |
| Chat | C |
| Help | F1 |
| Episode objective | F2 |
| Save game | F3 |
| Load game | F4 |
| Chat (alt) | F5 |


## 🔨 Build From Source

### Requirements

- [Visual Studio 2022](https://visualstudio.microsoft.com/)
- **Desktop development with C++** workload
- **MFC for v143 build tools** (x86 and x64)
- A compatible Windows SDK

### Steps

```
1. Open  src/Murk.sln  in Visual Studio 2022
2. Select  Release | Win32  configuration
3. Build the solution  (Ctrl+Shift+B)
4. Output:  bin/Murk.exe
```

## 🔬 Technical Deep Dive

This repository is a time capsule of **late-1990s Windows game programming** — before Unity, Unreal, and modern engines took over. It's a valuable reference for:

| Area | What you'll find |
| :--- | :--- |
| **DirectDraw rendering** | Software-driven 2D isometric blitting and sprite management |
| **DirectSound + MIDI** | Sound effect playback and MIDI music streaming |
| **Win32 architecture** | Message loop, window management, GDI interop |
| **MFC integration** | Dialog-based UI, resource management, application framework |
| **DirectPlay networking** | Session creation, player management, lobby code |
| **Game logic** | Dungeon generation, AI, collision, item systems in plain C++ |

## 📁 Repository Layout

```
Murk/
├── src/       C++ source, headers, VS solution, resources, MIDI files
├── bin/       Pre-built binary, config files, save data, docs
├── artwork/   Screenshots, scans, promotional art
├── sound/     Audio assets (WAV)
├── LICENSE    MIT License
└── README.md
```

## 🌍 Historical Context

Murk was developed during the golden age of DirectX game programming, when indie developers shipped custom engines directly on top of the Windows 95 Game SDK. This codebase is a snapshot of that era — useful for **retro game preservation**, **game history research**, and anyone curious about how PC games were built before modern frameworks existed.

<p align="center">
  <img src="artwork/Windows95GameSDK.jpg" alt="Windows 95 Game SDK box" width="300">
  &nbsp;&nbsp;&nbsp;
  <img src="artwork/DirectX.jpg" alt="Classic DirectX packaging" width="300">
</p>

## 👤 Credits

- **Programming** — Mark Longo
- **Graphics** — Tom Portaccio

## 📜 License

This project is released under the **MIT License**. See [LICENSE](LICENSE) for details.

---

<p align="center">
  <sub>If you find Murk interesting, consider giving it a ⭐ — it helps others discover this piece of gaming history.</sub>
</p>

