# Xbox 360 Native 240p DashLaunch Plugin (`t240.xex`)

An experimental DashLaunch system plugin (`sysdll`) for RGH/JTAG Xbox 360 consoles (kernel **17559**) that forces **true 240p non-interlaced analog video output** on CRTs by patching kernel display tables, programming HANA/ANA registers, and reconfiguring Xenos CRTC/D1 MMIO registers at runtime.

> **Status:** Experimental proof-of-concept. Successfully outputs real 240p on a CRT (with no prior public implementation known), but it has **important known bugs** (see below) and targets hardcoded **17559** kernel addresses. Contributions and feedback from the Xbox 360 / CRT community are very welcome!

---

## Important: Is `Main.lua` Enough?

**No.** `aurora_script/Toggle_240p/Main.lua` is **only an optional Aurora utility menu** that writes `cmd.txt` (`t`, `1`, `0`) and reads `state.txt`.
- **The actual 240p kernel/hardware switching is done entirely by `t240.xex`** (compiled from `src/t240.c`), loaded at boot via DashLaunch (`Hdd:\launch.ini`).
- **No NAND flash or internal Xbox 360 system files are permanently modified.** Everything is temporary in RAM and controlled by `t240.xex` + `launch.ini` (+ optional `Main.lua`).

---

## Known Bugs & Critical Warnings

Please read these before testing on your console:

1. **Activate `LT + RT + L3 + R3` (`L2 + R2 + L3 + R3`) INSIDE a game or emulator — NOT in Aurora:**
   - Although `LT + RT + L3 + R3` activates 240p while sitting in Aurora, **once 240p is active in Aurora, you will not be able to launch any game**.
   - **Recommended workflow:** Boot into Aurora in `480i`, launch your game or emulator first, and **then** hold **`LT + RT + L3 + R3`** once the game/emulator is open.
2. **Console cannot shut down normally after enabling 240p:**
   - Once 240p has been activated, the console **cannot power off cleanly** via the normal shutdown flow — you must **unplug / hard-disconnect power** to turn it off.
3. **Not 100% infallible across title/video mode changes:**
   - Games or apps that re-initialize video modes (`VdSetDisplayMode` / `AvSetDisplayMode`) can reset HANA/CRTC back to `480i`, requiring `LT + RT + L3 + R3` again.
4. **Hardcoded Kernel `17559` Addresses & 480i Base:**
   - Your console must be on **kernel 17559** and set to **480i** standard-definition analog output before enabling 240p.

---

## Installation & Testing

1. Copy `prebuilt/t240.xex` (or your compiled `build/t240.xex`) to:
   `Hdd1:\AuroraLaunch\Plugins\t240.xex`
2. Add it to `Hdd1:\launch.ini` under `[Plugins]`, for example:
   ```ini
   plugin3 = Hdd:\AuroraLaunch\Plugins\t240.xex
   ```
3. *(Optional)* Copy `aurora_script/Toggle_240p/Main.lua` to:
   `Hdd1:\AuroraLaunch\User\Scripts\Utility\Toggle_240p\Main.lua`
4. Reboot the Xbox 360 (in `480i` analog output), open a game or emulator, and hold **`LT + RT + L3 + R3`** (`L2 + R2 + L3 + R3`) to switch to **240p**.

---

## How It Works (`src/t240.c`)

When `enable_240p()` is triggered:

1. **Kernel RAM Patches (`g_patches_240p` / `g_patches_480i`):**
   Patches 7 words in kernel `17559` with full cache coherency (`dcbst` + `icbi` + `isync`):
   - `0x800582C8` (`0x2611920C` for 240p / `0x2651B20D` for 480i)
   - `0x800582CC` (`0x23232323` for 240p / `0x67672323` for 480i)
   - `0x8005968C` (`0x83814068` for 240p / `0x8381406A` for 480i)
   - `0x80059694` (`0x10B420FE` for 240p / `0x10C420FF` for 480i)
   - `0x800D04C0` (`0x392A0000` for 240p / `0x392A0001` for 480i)
   - `0x800CF540` (`0x60000000` nop / `0x81610050`)
   - `0x800CF544` (`0x60000000` nop / `0x556B4B6C`)
2. **Dual HANA/ANA Writes (`hana_write_both`):**
   Writes ANA registers `0x15`, `0x16`, `0x26`, `0x28` via both `VdWriteAna` (`0x800BFA50`) and `CGpuWrite` (`0x800C0190`), toggling sync bits at `0x7FC87938`, `0x7FC87910`, `0x7FC87900`.
3. **Xenos CRTC / D1 MMIO (`0x7FC80000 + offset`):**
   Switches D1 out of interlaced `480i` (`0x6010 = 0x20C`) into non-interlaced `240p` (`0x6010 = 0x20B`), matching even/odd field counters (`0x6538`) and vertical init (`0x65DC = 0x65C8`).
4. **Dynamic Input Polling:**
   Resolves `XamInputGetState` dynamically via `XexGetModuleHandle("xam.xex")` + ordinal `401` so `t240.xex` keeps zero static `xam.xex` imports as a `<sysdll/>`.

---

## Building from Source

Requires the **Microsoft Xbox 360 SDK (XEDK)** installed at `C:\Program Files (x86)\Microsoft Xbox 360 SDK`:

```bat
build.bat
```

Outputs `build\t240.xex` (linked with `/DLL /BASE:0x81E00000` and `src\xex.xml`).

---

## Help Wanted (Community / Reddit)

1. Fixing the **shutdown / power-off hang** after 240p is enabled (likely related to one of the kernel patches or HANA/CRTC state during `HalReturnToFirmware`).
2. Fixing **Aurora game launching while 240p is active** (or cleanly hooking `VdSetDisplayMode` / `AvSetDisplayMode` so mode transitions don't break launches or revert to `480i`).
3. Replacing hardcoded **17559** addresses with dynamic signature/pattern scanning for multi-kernel support.
