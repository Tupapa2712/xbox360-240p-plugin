<p align="center">
  <img src="240p-dream/Media/banner.png" alt="240p Dream by tupapa2712 Banner" width="760" />
</p>

<h1 align="center">Xbox 360 Native 240p — <code>t240.xex</code> (v2.30) &amp; 240p Dream</h1>

<p align="center">
  <strong>True 240p analog CRT output for Xbox 360 RGH + Plug-and-Play Retro Emulator</strong><br/>
  Created by <strong>tupapa2712</strong>
</p>

<p align="center">
  <em>"No matter how much others rely on logic, data, truth, and knowledge, I keep believing in myself."</em>
</p>

<p align="center">
  <img src="240p-dream/Media/cover_front.png" alt="240p Dream Front Cover (by tupapa2712)" width="360" />
  &nbsp;&nbsp;&nbsp;
  <img src="240p-dream/Media/cover_back.png" alt="240p Dream Back Cover" width="360" />
</p>

---

## What is this?

Normally, when you connect an Xbox 360 to a standard-definition tube TV (CRT) using a Component, SCART, or AV cable, the console only outputs a flickering **480i** image.

This free, open-source project unlocks **real 240p output** on Xbox 360 RGH — giving you authentic scanlines, zero flicker, and razor-sharp pixels so retro emulators and Xbox 360 / Arcade games look just like classic consoles on a CRT.

---

## What's New in v2.30

### 240p Dream (Based on Salvia 360 v1.1.2)
- **Built-in BIOS Downloader & Updated Cores:** Includes a built-in **BIOS downloader** plus updated cores (**Snes9x** with MSU-1 audio, **Atari 2600 / Stella 2014**, **blueMSX**, and **mGBA**), ready for CRT out of the box in **`2560x240`** with crisp pixels.
- **Core Options Before You Play:** Configure **Core Options** for all **10 main cores** (PlayStation, Nintendo 64, Snes9x, Snes9x 2010, mGBA, Genesis Plus GX, Nestopia, Gambatte, Stella 2014, and blueMSX) directly from the crisp **480i main menu (`Options → Core Options`)** before launching a game — or in-game anytime with **`Select + Y`**.
- **PlayStation Controller Choice & Vibration:** Choose between **Standard (Digital)** and **DualShock** controllers for PlayStation 1 right from Core Options, and enjoy **controller vibration (Rumble)** in both **PlayStation 1** (with DualShock) and **Nintendo 64**, with automatic vibration stop when pausing or exiting.
- **Improved Nintendo 64 Display:** N64 games now display centered, full-screen, and razor-sharp in `2560x240`.
- **Instant Exit to 480i Menu:** Press **`LB + RB + LT + RT`** inside any retro game to exit straight back to the main menu cleanly in **480i**.

### `t240.xex` Plugin (Native Xbox 360 & XBLA Games)
- **Direct `2560x240` with One Combo:** No external scripts, selectors, or popups needed. Just press **`LT + RT + L3 + R3`** inside any Xbox 360 or Arcade game to switch between **`2560x240`** and **`480i`**.
- **Clean & Lag-Free Switching:** Smooth, stable return to **480i** with zero on-screen popups and zero added input lag.

---

## Easy Setup & How to Use

### What You Need
* **Xbox 360 with RGH** (Kernel **17559**)
* **CRT TV** connected via S-video, Component, SCART, or AV cable (set Xbox display to **480i**)
* **DashLaunch** and **Aurora Dashboard**

> **IMPORTANT — Not Compatible with Stealth Servers:** `t240.xex` is **NOT compatible with Xbox Live Stealth servers**. Do not load it alongside any Stealth server plugin in `launch.ini`. This project is for offline RGH/JTAG CRT use only.
>
> **Note:** No copyrighted ROMs or BIOS files are included in the ZIP. You can use the built-in BIOS downloader in `240p Dream` or add your own legally dumped games and BIOS files.

### 3-Step Installation
1. **Download the files** from the [Releases](https://github.com/Tupapa2712/xbox360-240p-plugin/releases) page:
   * `t240.xex`
   * `240p-Dream-v2.30-Complete-NoROMs-NoBIOS.zip`
2. **Install `t240.xex`:**
   * Copy `t240.xex` to your console (e.g., `Hdd:\AuroraLaunch\Plugins\t240.xex`).
   * Open **DashLaunch** (or edit `Hdd:\launch.ini`) and assign a free plugin slot: `plugin3 = Hdd:\AuroraLaunch\Plugins\t240.xex`
   * Save and **reboot** your console.
3. **Install `240p Dream`:**
   * Extract `240p Dream` to your emulators folder (e.g., `Hdd:\Emulators\240p Dream\`).
   * Drop your games into the pre-created folders under `240p Dream\roms\<system>\`, and use the built-in BIOS downloader (or copy RetroArch BIOS files into `240p Dream\system\`).
   * Scan your content paths in **Aurora** — cover art, banner, background, and icon load automatically.

### Quick Controls
| Where | How it Works | Button Combo |
| :--- | :--- | :--- |
| **240p Dream (Emulators)** | **100% Automatic.** Menus stay in clear **480i**; games switch automatically to **240p**. | **`LB + RB + LT + RT`** *(exit game & return to 480i menu)* |
| **240p Dream (In-Game Menu)** | Open the in-game menu anytime (or configure Core Options in 480i before playing). | **`Select + Y`** *(open/close in-game menu)* |
| **Native Xbox 360 / XBLA** | Launch your game from Aurora in **480i**, then toggle in-game. | **`LT + RT + L3 + R3`** *(toggle **`2560x240`** ↔ **`480i`**)* |

### Important Tips for Players
1. **Xbox 360 & XBLA loading screens, menus & level changes:**
   - Start native Xbox 360 and XBLA games from Aurora in **480i**, then press **`LT + RT + L3 + R3`** once you are in the game.
   - **If a game gets stuck on a loading screen, menu, or level transition while in 240p:** press **`LT + RT + L3 + R3`** to switch back to **480i** so it can move forward, and press **`LT + RT + L3 + R3`** again once gameplay continues.
2. **Not Compatible with Stealth Servers:** Make sure Stealth plugins are disabled in `launch.ini` when using `t240.xex`.
3. **Turning Off Safely:** Switch back to **480i** before shutting down; if you use the physical power button while in 240p, wait 5–10 seconds for the console to restore 480i and power off cleanly.

---

## Official v2.30 SHA-256 Checksums

<details>
<summary><strong>Click to expand: Official v2.30 SHA-256 Checksums</strong></summary>

| File | Size | SHA-256 |
|---|---|---|
| `t240.xex` (v2.30) | `102,400` B | `BF428062E2BAB19A144B117F051BCC5AB682240CF2C5F3BF1155808B129F7C50` |
| `240p-Dream-v2.30-Complete-NoROMs-NoBIOS.zip` | `292,170,172` B | `CF8A9C62FC3A51BF6C4981682449C7A08E76B1FA52FB8E3099A4EF00318DF5A7` |
| `default.xex` (240p Dream) | `8,134,656` B | `5A9A33B7F6126CF9BAFBB313ED4E98BF7BF38C60391A6D157719AA075BC2B52E` |
| `pcsxr-360.xex` (PlayStation) | `23,425,024` B | `26EAC3B3CFA83DE343EBBCFE6183D024BAFE9394D7EEF526F1E3281B2CF82884` |
| `xbox64.xex` (Nintendo 64) | `9,244,672` B | `7D69B53C23E0CFD3E057AD0D141E3BBF825AA1F9042FAA7EA04819B8A17AFF5B` |
| `Snes9x.xex` (SNES MSU-1) | `10,371,072` B | `D9797172C6CFD096E342E3DF578EECB6DBA06CF76E295C28DA136DCB7429CFE1` |
| `Snes9x2010.xex` (SNES) | `10,432,512` B | `B8999CD6E281F287F0C93224EC49461AE141FD05EACE3B2B97A48273C5A33B4C` |
| `Nestopia.xex` (NES) | `11,317,248` B | `C7F87429D0FFAA798DA67FD8D3509F0C298963F2F32982B2F31275449E419958` |
| `Genesis-Plus-GX.xex` (Genesis/MD) | `12,378,112` B | `6737B3FA20E0F9449BF24961DF6DE21F1E2C12BEA1B43083CAB2021BEF932523` |
| `mgba.xex` (GBA) | `9,908,224` B | `0C5CB158786403E195D34AC776C792B259D02114EF0878EC423AB70BC026ED23` |
| `Gambatte.xex` (GB/GBC) | `9,412,608` B | `B54BADC6E93D3A712EC2980BD96D83849630D561255CF795758BE33B5576A9BF` |
| `stella2014.xex` (Atari 2600) | `9,216,000` B | `0230FDFB64BFDE7285034DEEEFB2BB596FF949B158B52D7375544C53E30D82FE` |
| `blueMSX.xex` (MSX/MSX2) | `9,383,936` B | `7AAB323DB83C8F5FCAA4F212B09E1A0C6AEB8FDE65F0E909E43EB40FE5D9D5FC` |

</details>

---

## Building from Source

Requires the **Microsoft Xbox 360 SDK (XEDK)** and full **Visual Studio 2010** (Visual Studio Express is not supported):

```bat
build.bat
```

Outputs `build\t240.xex`.

## License & Credits

- **`t240.xex`:** Open-source Xbox 360 240p DashLaunch plugin by **tupapa2712**.
- **`240p Dream`:** CRT-focused fork of **[Salvia 360](https://github.com/danibiker/Salvia)** (v1.1.2) by **danibiker**, licensed under **GNU AGPL v3.0** (see `LICENSE` and `licenses/`).
