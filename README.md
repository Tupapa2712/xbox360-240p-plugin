<p align="center">
  <img src="240p-dream/Media/banner.png" alt="240p Dream by tupapa2712 Banner" width="760" />
</p>

<h1 align="center">Xbox 360 Native 240p — <code>t240.xex</code> (v2.22) &amp; 240p Dream</h1>

<p align="center">
  <strong>True 15 kHz non-interlaced 240p analog CRT output for Xbox 360 RGH + Plug-and-Play Native 240p Emulator Frontend</strong><br/>
  Developed by <strong>tupapa2712</strong>
</p>

<p align="center">
  <em>"Por más que otros se basen en la lógica, los datos, la verdad y el conocimiento, yo sigo creyendo en mi mismo"</em>
</p>

<p align="center">
  <img src="240p-dream/Media/cover_front.png" alt="240p Dream Front Cover (by tupapa2712)" width="360" />
  &nbsp;&nbsp;&nbsp;
  <img src="240p-dream/Media/cover_back.png" alt="240p Dream Back Cover" width="360" />
</p>

---

## Overview & What's New in v2.22

For nearly two decades, the Xbox 360 was considered unable to output a real 240p analog video signal. This open-source project combines two components to bring **authentic 15.734 kHz progressive 240p** to standard-definition CRT televisions:

1. **`t240.xex` (v2.22)** — A DashLaunch system plugin (`<sysdll/>`) for Kernel `17559` that patches kernel video tables, programs HANA/ANA registers, and reconfigures the Xenos CRTC / `D1SCL` hardware scaler at runtime.
2. **`240p Dream`** — An open-source fork and CRT-focused modification of **Salvia 360** (by **DaniBiker**) that renders classic consoles at genuine 240-line framebuffers and communicates automatically with `t240.xex` via an in-memory IPC beacon (`S240` v3).

### Key Highlights (v2.22)
- **True 240p Analog CRT Signal:** Real progressive 15 kHz scanlines on CRT TVs with **zero interlace flicker**.
- **Nearest-Neighbor Hardware Sharpness:** Programs Xenos `D1SCL` scaling LUTs to unity weights (`w = 512`), eliminating the muddy 480i deflicker blur while in 240p (and cleanly restoring 50/50 deflicker when returning to 480i).
- **Zero Vertical Bounce / Jitter:** Phase-aligned `0x65DC = 0x65C8 - 0x00010000` eliminates the 1-line vertical bounce on progressive scanlines.
- **Clean 240p <-> 480i Toggling (No More Vertical Color Bars):** Keeping `D1MODE_DATA_FORMAT` (`0x6528 = 0`) across both enable and disable allows switching cleanly back to 480i inside retail Xbox 360 / XBLA games without freezing the vertical scaler.
- **Full 4:3 Aspect Ratio on SNES & NES (No Black Sidebars):**
  - `Snes9x2010.xex`, `Snes9x.xex`, and `Nestopia.xex` automatically run a **`640x240`** backbuffer (keeping `src_w <= 640` so `t240.xex` preserves Nearest-Neighbor in Xenos) and fill **100% of the 4:3 horizontal width** while keeping vertical resolution strictly **1:1 (240p native scanlines)**.
  - Includes a built-in full-precision `ps_3_0` **Sharp-Bilinear-X / Point-Y** shader that eliminates horizontal scrolling shimmer across 256px -> 640px (and SNES 512px Hi-Res -> 640px) with zero vertical scanline bleed.
- **Plug-and-Play Aurora Integration (Frutiger Aero Style):**
  - Ships with `240p Dream/Media/` containing the full 3D box cover (`cover.png`), front & back covers (`cover_front.png`, `cover_back.png`), 16:9 banner (`banner.png`), 720p panoramic wallpaper (`background.png`), 128x128 crystal icon (`icon.png`), and `default.xml` metadata for automatic Aurora detection.

---

## Disclaimer (Use at Your Own Risk)

- This plugin modifies kernel RAM, HANA/ANA registers, and Xenos CRTC/D1 MMIO registers at runtime in RAM (**no NAND flash or permanent system files are modified**).
- **Tested on:** Xbox 360 S (Model 1439, Corona `12V 9.6A`, RGH, Kernel `17559`) connected to an SD CRT TV (Sony Trinitron WEGA) over **Component (YPbPr)** set to **480i**.
- Not tested over HDMI or on kernels other than `17559`. Use strictly at your own risk and keep a backup of your `launch.ini`.

---

## Requirements

- **Xbox 360 with RGH / JTAG** running **Kernel `17559`**
- **SD Analog CRT TV (15 kHz)** connected via **Component (YPbPr)** or **AV Composite/RGB** cable
- Xbox 360 system display settings configured to **480i**
- **DashLaunch** (to load `t240.xex` at boot)
- **Aurora Dashboard**

---

## Step-by-Step Installation

> **Legal Notice:** Neither this repository nor the GitHub Releases include any copyrighted game ROMs, console BIOS files, or Microsoft Xbox 360 SDK files. You must provide your own legally dumped ROMs and BIOS files.

### 1. Install the `t240.xex` (v2.22) Plugin
1. Download `prebuilt/t240.xex` (or the `v2.22` release archive from **GitHub Releases**).
2. Copy `t240.xex` to your Xbox 360 internal HDD:
   ```text
   Hdd:\AuroraLaunch\Plugins\t240.xex
   ```
3. Open **DashLaunch** on the console (or edit `Hdd:\launch.ini`) and assign `t240.xex` to a free plugin slot:
   ```ini
   plugin3 = Hdd:\AuroraLaunch\Plugins\t240.xex
   ```
4. Save `launch.ini` to `Hdd:` and **cold reboot** the console.
5. *(Optional)* Copy `aurora_script/Toggle_240p/Main.lua` to `Hdd:\AuroraLaunch\User\Scripts\Utility\Toggle_240p\Main.lua`.

### 2. Install `240p Dream` (Plug-and-Play with Aurora)
1. Download the `240p Dream` release package from **GitHub Releases** (which includes the updated `Snes9x2010.xex`, `Snes9x.xex`, `Nestopia.xex`, `salvia.cfg`, `default.xml`, and `Media/`).
2. Extract the folder to your console HDD:
   ```text
   Hdd:\Emulators\240p Dream\
   ├── default.xex
   ├── Snes9x2010.xex
   ├── Snes9x.xex
   ├── Nestopia.xex
   ├── Genesis-Plus-GX.xex
   ├── ... (other libretro cores)
   ├── salvia.cfg
   ├── default.xml
   ├── plugin\t240.xex
   ├── config\
   └── Media\
       ├── cover.png
       ├── cover_front.png
       ├── cover_back.png
       ├── banner.png
       ├── background.png
       ├── icon.png
       └── default.xml
   ```
3. Place your own ROMs inside `Hdd:\Emulators\240p Dream\roms\<system>\` (and any required BIOS files inside `Hdd:\Emulators\240p Dream\system\`).
4. Scan your emulators path in **Aurora** — Aurora will pick up **240p Dream** with its Frutiger Aero artwork and metadata automatically (precompiled `.asset` files are also included inside the release archive under `Media/Aurora_Asset_Files/` if you want to drop them directly into Aurora's `Data/GameData/`).

---

## Usage Guide

### Emulators (`240p Dream`) — 100% Automatic
1. Launch **240p Dream** from Aurora. The main frontend menu (`default.xex`) opens cleanly in **480i** so all text and menus stay razor-legible on a CRT.
2. Select and launch any game (SNES, NES, Mega Drive / Genesis, etc.). The core signals `t240.xex` via the `S240` v3 beacon and automatically switches the console to **native 240p** with real scanlines, Nearest-Neighbor sharpness, zero vertical bounce, and full 4:3 width on SNES & NES.
3. To exit the game and return to the `240p Dream` menu, press **`LB + RB + Select` (Back)**. The core automatically restores **480i** cleanly before reloading the menu.

### Retail Xbox 360 & XBLA Games — Manual Hotkey Toggle
1. **Always keep Aurora in 480i.** Launch your Xbox 360 retail or XBLA game normally from Aurora while in 480i.
2. Once inside the game, hold **`LT + RT + L3 + R3`** (`L2 + R2 + Left Stick + Right Stick`) to toggle **240p ON**.
3. Hold **`LT + RT + L3 + R3`** again at any time to return cleanly to **480i** (in v2.22 returning to 480i works smoothly with no freezes or vertical bar corruption, letting you open the Guide menu to exit or power off normally).

---

## Known Limitations (v2.22)

1. **Always keep Aurora in 480i when launching retail/XBLA games:** Activating 240p while sitting in Aurora and then trying to launch a retail/XBLA title will freeze on the loading screen. Enter the game in 480i first, then toggle `LT + RT + L3 + R3`.
2. **Xbox Guide button while 240p is active:** The system Guide blade will not open while 240p mode is engaged. Return to 480i first with **`LT + RT + L3 + R3`** (or **`LB + RB + Select`** inside `240p Dream`) before pressing Guide — or press the console's physical power button and wait **5–10 seconds** while `t240.xex` safely restores 480i and completes shutdown.

---

## Official v2.22 SHA-256 Checksums

| File | Size | SHA-256 |
|---|---|---|
| `prebuilt/t240.xex` (v2.22) | `102,400` B | `AA19E9EFD40C7AEFD0FD1076BF29F2B16D958B3956157505063332117A36DDE5` |
| `240p-dream/prebuilt/Snes9x2010.xex` | `9,351,168` B | `85C9775F97F7A950449806A52FE9B9F5EF4BA7C894F8EA7CE2BE80BD9B118D6E` |
| `240p-dream/prebuilt/Snes9x.xex` | `9,289,728` B | `2E97406630C58636F119E59F13A401EBF45DFBADF3E24272CD2A3581D8990AFD` |
| `240p-dream/prebuilt/Nestopia.xex` | `10,235,904` B | `5AF88FFB055656CB49C5D3BE6A30AC48F3051F9898397C7476AAE4551E007689` |

---

## Repository Structure

```text
.
├── .github/FUNDING.yml              # GitHub Sponsor / Ko-fi funding link
├── LICENSE                          # Original Salvia 360 GNU AGPL v3.0 license (preserved verbatim)
├── licenses/                        # Preserved original licenses for Salvia 360 & all libretro cores
├── README.md                        # English project documentation & visual preview
├── GUIA_USUARIO_GITHUB_REDDIT.txt   # Spanish step-by-step user guide
├── build.bat                        # Build script for t240.xex (requires your own XEDK + full VS2010)
├── src/
│   ├── t240.c                       # Complete v2.22 plugin source code
│   └── xex.xml                      # XEX sysdll configuration
├── prebuilt/
│   └── t240.xex                     # Verified v2.22 plugin binary
├── aurora_script/
│   └── Toggle_240p/Main.lua         # Optional Aurora utility script
└── 240p-dream/
    ├── prebuilt/                    # Verified 4:3 240p SNES & NES cores (Snes9x2010, Snes9x, Nestopia)
    ├── src/                         # Clean 240p Dream C++ frontend source code (Salvia 360 fork)
    ├── libs/libSDLx360/SDL/src/video/xbox/
    │   ├── SDL_xboxvideo.c          # Xbox 360 D3D9 240p driver, S240 v3 beacon & 4:3 CRT quad math
    │   └── SDL_shaders_src.h        # Built-in Sharp-Bilinear-X / Point-Y ps_3_0 HLSL shader
    ├── config/                      # Clean emulator configuration templates
    ├── assets/shaders/              # Nearest passthrough shader preset
    ├── salvia.cfg                   # Default 240p CRT configuration (zero user credentials)
    ├── retropad.ini                 # Default controller mapping
    ├── default.xml                  # Local Aurora metadata
    └── Media/                       # Frutiger Aero Aurora artwork (cover, front, back, banner, bg, icon)
```

---

## Building from Source

> You do **not** need to compile from source to use this project — ready-to-run binaries are available in `prebuilt/t240.xex` and **GitHub Releases**.

To compile `t240.xex` or the `240p Dream` cores yourself, you must supply your own **Microsoft Xbox 360 SDK (XEDK)** and full **Visual Studio 2010** (Visual Studio Express editions do not include the PowerPC toolchain). No proprietary SDK files are included in this repository.

```bat
build.bat
```

---

## Credits & Attribution

- **Salvia 360 (Original Project):** **240p Dream** is an open-source fork and CRT-specialized modification of [**Salvia 360**](https://github.com/danibiker/Salvia) created by **DaniBiker**. All credit for the original Xbox 360 Libretro frontend architecture goes to DaniBiker. The original `LICENSE` file from Salvia 360 is preserved verbatim at the root of this repository and in `licenses/`.
- **Libretro Cores & Upstream Authors:** Huge thanks to the **Libretro / RetroArch** team and the original authors and Xbox 360 porters of every core included in `240p Dream` — including **Snes9x2010**, **Snes9x**, **Nestopia**, **Genesis Plus GX**, **PicoDrive**, **Gambatte**, **VBA-Next**, **FinalBurn Neo (FBNeo)**, **FBANext**, **MAME 2003-Plus**, **Beetle / Mednafen** (PCE, PCE Fast, SuperGrafx, Lynx, NGP, VB, WonderSwan), **PCSXR-360**, **Xbox64 / Wii64**, **DOSBox-Pure**, **PUAE / PUAE2021**, **Frodo**, **Atari800**, **PX68k**, **PrBoom**, **TyrQuake**, **Cannonball**, and **3DOX**. Original core licenses are preserved verbatim in [`licenses/`](licenses/).
- **240p Hardware Research, `t240.xex` v2.22, & `240p Dream` CRT Fork:** **tupapa2712** (with AI-assisted engineering & testing).

---

## Support the Project

Everything in this project (`t240.xex` and `240p Dream`) is — and will always remain — **100% free and open-source**.

If this project brought crisp 240p scanlines back to your CRT and you'd like to buy me a coffee to support future native 240p CRT research on other consoles (**PS2, PS3, Original Xbox**), you can support my work here:

<p align="center">
  <a href="https://ko-fi.com/tupapa2712"><strong>Support tupapa2712 on Ko-fi (ko-fi.com/tupapa2712)</strong></a>
</p>
