<p align="center">
  <img src="240p-dream/Media/banner.png" alt="240p Dream by tupapa2712 Banner" width="760" />
</p>

<h1 align="center">Xbox 360 Native 240p — <code>t240.xex</code> (v2.22) &amp; 240p Dream</h1>

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

Normally, when you connect an Xbox 360 to a standard-definition tube TV (CRT) using a Component, SCART, or AV cable, the console only outputs a flickering **480i** interlaced image.

This free, open-source project unlocks **real 240p progressive output** on Xbox 360 RGH — eliminating interlace flicker and delivering authentic black scanlines and razor-sharp pixels, so 8-bit/16-bit emulators and 2D Xbox 360 games look just like original retro consoles on a CRT.

---

## What's Included (v2.22)

- **240p Dream (Retro Emulator):** A CRT-focused modification of **Salvia 360** that automatically switches your TV to true **240p** the moment you start a retro game — and cleanly switches back to **480i** when you return to the menu (`LB + RB + Select`).
  
- **`t240.xex` Plugin (For Native Xbox 360 & XBLA Games):** Great for 2D and arcade Xbox 360 titles. Just launch your game normally from Aurora and press **`LT + RT + L3 + R3`** in-game to toggle 240p on or off whenever you want.
- **Ready for Aurora:** Includes matching Frutiger Aero artwork (front cover, back cover, full 3D box cover, banner, wallpaper, icon, and English metadata) so Aurora displays everything cleanly right after scanning.

---

## Easy Setup & How to Use

### What You Need
* **Xbox 360 with RGH** (Kernel 17559)
* **CRT TV** connected via Component, SCART, or AV cable (set Xbox display to **480i**)
* **DashLaunch** and **Aurora Dashboard**

> **Note:** No copyrighted ROMs, BIOS files, or Xbox 360 SDK files are included. Bring your own legally dumped games and BIOS files.

### 3-Step Installation
1. **Download both files** from the [Releases](https://github.com/Tupapa2712/xbox360-240p-plugin/releases) page:
   * `t240-v2.22-plugin.zip`
   * `240p-Dream-v2.22-Complete-NoROMs-NoBIOS.zip`
2. **Install the `t240.xex` plugin:**
   * Copy `t240.xex` to your console (e.g., `Hdd:\AuroraLaunch\Plugins\t240.xex`).
   * Open **DashLaunch** (or edit `Hdd:\launch.ini`) and add it to a free plugin slot: `plugin3 = Hdd:\AuroraLaunch\Plugins\t240.xex`
   * Save and **reboot** your console.
3. **Install `240p Dream`:**
   * Copy the `240p Dream` folder to your emulators directory (e.g., `Hdd:\Emulators\240p Dream\`).
   * **Important — BIOS & ROMs are NOT included:** You must obtain your own BIOS files and games separately. `240p Dream` uses the **exact same standard BIOS files as RetroArch** — place required BIOS files inside `240p Dream\system\` (see **`BIOS_LIST.txt`** inside the emulator ZIP for the full list per system) and drop your games into the pre-created empty folders inside `240p Dream\roms\<system>\`.
   * Scan your content paths in **Aurora** — box art, banner, background, icon, and description will load automatically.

### Quick Controls
| Where | How it Works | Button Combo |
| :--- | :--- | :--- |
| **240p Dream (Emulators)** | **100% Automatic.** Menus stay in readable 480i; games launch directly in 240p. | `LB + RB + Select` *(exit game & return to 480i menu)* |
| **Native Xbox 360 / XBLA** | Launch the game normally from Aurora in 480i, then toggle in-game. | `LT + RT + L3 + R3` *(toggle 240p <-> 480i anytime)* |

### Two Important Rules (v2.22 Limitations)
1. **Always keep Aurora in 480i when launching native Xbox 360 / XBLA games.** Launching a retail or XBLA game while 240p is already active will freeze the loading screen. Enter the game in 480i first, then press `LT + RT + L3 + R3`.
2. **Switch back to 480i before using the Xbox Guide button.** The Guide menu won't open while 240p is active. Toggle back to 480i first (`LT + RT + L3 + R3`, or `LB + RB + Select` in `240p Dream`) to exit or shut down normally. If you press the console's physical power button while in 240p, wait **5–10 seconds** while the plugin safely restores 480i and powers down.

---

## Technical Details, Checksums & Building from Source

<details>
<summary><strong>Click to expand: How it works under the hood (15 kHz HANA/Xenos, Anti-Bounce & 4:3 Shader)</strong></summary>

- **Safety & Scope:** All changes happen in RAM at runtime — **no NAND flash or permanent system files are modified**. Tested on an **Xbox 360 S (Model 1439, Corona `12V 9.6A`, RGH, Kernel `17559`)** over Component (YPbPr) into a Sony Trinitron WEGA SD CRT. Not tested over HDMI or on kernels other than `17559`. Use at your own risk.
- **True 15.734 kHz 240p Signal (`t240.xex` v2.22):**
  - DashLaunch `<sysdll/>` worker running at elevated priority (`KeSetPriorityThread(..., 20)`) on system processor `0x02`.
  - Patches Kernel `17559` video tables and programs HANA/ANA (`0x15 = 0x2601820C`, `0x16 = 0x23232323`, `0x26 = 0x83814068`, `0x28 = 0x10B420FE`) via dual `VdWriteAna` + `CGpuWrite` + commit `0x5E`.
  - Reconfigures Xenos CRTC/D1 MMIO (`0x6010 = 0x20B`, `0x6030 = 1`, `0x6528 = 0` on both enable and disable to prevent vertical color bars when returning to 480i, `0x6538 = (d<<16)|d`).
  - **Zero vertical bounce:** Applies phase offset `0x65DC = 0x65C8 - 0x00010000`.
  - **Nearest-Neighbor hardware LUTs:** Programs unity weights (`w = 512`) into Xenos `D1SCL` vertical (and horizontal when `src_w <= 640`) banks while in 240p, and restores 50/50 vertical deflicker on return to 480i.
- **Automatic Emulator IPC (`S240` v3 Beacon):** `240p Dream` exposes a volatile `S240` beacon in `.data` so `t240.xex` automatically switches between 480i (`default.xex`) and 240p (game cores).
- **Full 4:3 SNES & NES Without Shimmer (`Snes9x2010`, `Snes9x`, `Nestopia`):**
  - Upgrades backbuffer width to **`640x240`** (keeping `src_w <= 640` so Xenos keeps Nearest-Neighbor LUTs active) with `D3DPRESENTFLAG_NO_LETTERBOX` edge-to-edge 4:3 mapping and strict **1:1 vertical scanlines** (`224`/`239`/`240` lines unscaled).
  - Uses a built-in full-precision `ps_3_0` **Sharp-Bilinear-X / Point-Y** HLSL shader whenever horizontal scaling is fractional (`256 -> 640` at `2.5x`, or SNES Hi-Res `512 -> 640` at `1.25x`), removing horizontal scroll shimmer with zero vertical scanline bleed.

</details>

<details>
<summary><strong>Click to expand: Official v2.22 SHA-256 Checksums</strong></summary>

| File | Size | SHA-256 |
|---|---|---|
| `prebuilt/t240.xex` (v2.22) | `102,400` B | `AA19E9EFD40C7AEFD0FD1076BF29F2B16D958B3956157505063332117A36DDE5` |
| `240p-dream/prebuilt/Snes9x2010.xex` | `9,351,168` B | `85C9775F97F7A950449806A52FE9B9F5EF4BA7C894F8EA7CE2BE80BD9B118D6E` |
| `240p-dream/prebuilt/Snes9x.xex` | `9,289,728` B | `2E97406630C58636F119E59F13A401EBF45DFBADF3E24272CD2A3581D8990AFD` |
| `240p-dream/prebuilt/Nestopia.xex` | `10,235,904` B | `5AF88FFB055656CB49C5D3BE6A30AC48F3051F9898397C7476AAE4551E007689` |
| `t240-v2.22-plugin.zip` | `115,467` B | `54F50DBEF1C7B5999E2705E15CB84B0245D5F3145FCC19FAB7F835FC540DF81D` |
| `240p-Dream-v2.22-Complete-NoROMs-NoBIOS.zip` | `375,403,051` B | `18072F8208FC3562A1D50E9B80FC3299758DFDAC564408748646F38BFA7EDA89` |

</details>

<details>
<summary><strong>Click to expand: Repository Structure & Building from Source</strong></summary>

```text
.
├── .github/FUNDING.yml              # GitHub Sponsor / Ko-fi link
├── LICENSE                          # Original Salvia 360 GNU AGPL v3.0 license
├── licenses/                        # Preserved original licenses for Salvia 360 & all libretro cores
├── README.md                        # English project documentation
├── GUIA_USUARIO_GITHUB_REDDIT.txt   # Spanish step-by-step user guide
├── build.bat                        # Build script for t240.xex
├── src/                             # t240.xex v2.22 source code (t240.c + xex.xml)
├── prebuilt/t240.xex                # Verified v2.22 plugin binary
├── aurora_script/Toggle_240p/       # Optional Aurora utility Lua script
└── 240p-dream/
    ├── prebuilt/                    # Verified 4:3 240p SNES & NES cores (Snes9x2010, Snes9x, Nestopia)
    ├── src/                         # Clean 240p Dream C++ frontend source code (Salvia 360 fork)
    ├── libs/libSDLx360/SDL/src/video/xbox/  # Xbox 360 D3D9 240p driver & Sharp-X/Point-Y HLSL shader
    ├── config/                      # Clean emulator configuration templates
    ├── assets/shaders/              # Nearest passthrough shader preset
    ├── salvia.cfg                   # Default English 240p CRT configuration
    ├── retropad.ini                 # Default controller mapping
    ├── default.xml                  # English local Aurora metadata
    ├── BIOS_LIST.txt                # Complete list of standard RetroArch BIOS files
    ├── roms/                        # Pre-created empty system subfolders (no ROMs included)
    ├── system/BIOS_LIST.txt         # BIOS reference list (no BIOS files included)
    └── Media/                       # Frutiger Aero Aurora artwork (cover, front, back, banner, bg, icon)
```

> You do **not** need to compile from source to use this project — ready-to-run downloads are in [**Releases**](https://github.com/Tupapa2712/xbox360-240p-plugin/releases).

To build from source yourself, you need your own **Microsoft Xbox 360 SDK (XEDK)** and full **Visual Studio 2010** (Express editions lack the PowerPC toolchain). No proprietary SDK files are included here.

```bat
build.bat
```

</details>

---

## Credits & Attribution

- **Salvia 360:** **240p Dream** is an open-source fork and CRT-specialized modification of [**Salvia 360**](https://github.com/danibiker/Salvia) created by **DaniBiker**. Full credit for the original Xbox 360 Libretro frontend belongs to DaniBiker, and its original `LICENSE` file is preserved verbatim in this repository.
- **Libretro Cores & Upstream Authors:** Thanks to the **Libretro / RetroArch** team and the authors/porters of all included cores — **Snes9x2010**, **Snes9x**, **Nestopia**, **Genesis Plus GX**, **PicoDrive**, **Gambatte**, **VBA-Next**, **FBNeo**, **FBANext**, **MAME 2003-Plus**, **Beetle / Mednafen**, **PCSXR-360**, **Xbox64 / Wii64**, **DOSBox-Pure**, **PUAE**, **Frodo**, **Atari800**, **PX68k**, **PrBoom**, **TyrQuake**, **Cannonball**, and **3DOX**. All original core licenses are preserved in [`licenses/`](licenses/).
- *Developed with technical assistance, debugging, and documentation support from Gemini AI.*

---

## Support the Project

Everything in this project (`t240.xex` and `240p Dream`) is — and will always remain — **100% free and open-source**.

If this brought authentic 240p scanlines to your CRT and you'd like to buy me a coffee to support future native 240p CRT research on other consoles (**PS3, Wii U, Original Xbox**), you can do so here:

<p align="center">
  <a href="https://ko-fi.com/tupapa2712"><strong>Support tupapa2712 on Ko-fi (ko-fi.com/tupapa2712)</strong></a>
</p>
