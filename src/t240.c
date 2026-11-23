/*
 * 240p Dream & t240.xex (v2.23)
 * Desarrollado por: tupapa2712
 * "Por más que otros se basen en la lógica, los datos, la verdad y el conocimiento, yo sigo creyendo en mi mismo"
 */

#include <xtl.h>
#include <ppcintrinsics.h>

typedef long NTSTATUS;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} ANSI_STRING, *PANSI_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    HANDLE RootDirectory;
    PANSI_STRING ObjectName;
    ULONG Attributes;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef union _LARGE_INTEGER_K {
    struct {
        ULONG LowPart;
        LONG HighPart;
    };
    LONGLONG QuadPart;
} LARGE_INTEGER_K, *PLARGE_INTEGER_K;

#ifdef __cplusplus
extern "C" {
#endif
VOID RtlInitAnsiString(PANSI_STRING DestinationString, const char* SourceString);
NTSTATUS NtOpenFile(PHANDLE FileHandle, ULONG DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes,
                    PIO_STATUS_BLOCK IoStatusBlock, ULONG ShareAccess, ULONG OpenOptions);
NTSTATUS NtCreateFile(PHANDLE FileHandle, ULONG DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes,
                      PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER_K AllocationSize, ULONG FileAttributes,
                      ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions);
NTSTATUS NtReadFile(HANDLE FileHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext,
                    PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER_K ByteOffset);
NTSTATUS NtWriteFile(HANDLE FileHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext,
                     PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER_K ByteOffset);
NTSTATUS NtClose(HANDLE Handle);
NTSTATUS KeDelayExecutionThread(ULONG WaitMode, BOOLEAN Alertable, PLARGE_INTEGER_K Interval);
NTSTATUS ExCreateThread(PHANDLE ThreadHandle, ULONG StackSize, PULONG ThreadId,
                        PVOID ApiThreadStartup, PVOID StartRoutine, PVOID Parameter, ULONG CreationFlags);
VOID XapiThreadStartup(VOID (*StartRoutine)(PVOID), PVOID StartContext);
NTSTATUS XexGetModuleHandle(const char* ModuleName, PHANDLE ModuleHandle);
NTSTATUS XexGetProcedureAddress(HANDLE ModuleHandle, ULONG Ordinal, PVOID* Address);
BOOLEAN MmIsAddressValid(PVOID VirtualAddress);
VOID HalReturnToFirmware(ULONG Routine);
VOID HalSendSMCMessage(PVOID Input, PVOID Output);
NTSTATUS ObReferenceObjectByHandle(HANDLE Handle, PVOID ObjectType, PVOID* Object);
VOID ObDereferenceObject(PVOID Object);
LONG KeSetPriorityThread(PVOID Thread, LONG Priority);
#ifdef __cplusplus
}
#endif

typedef DWORD (*FN_XAM_INPUT_GET_STATE)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_STATE* pState);

typedef VOID  (*FN_VD_WRITE_ANA)(DWORD reg, DWORD val);
typedef VOID  (*FN_CGPU_WRITE)(DWORD reg, DWORD val);
typedef DWORD (*FN_MMIO_RD)(DWORD addr);
typedef VOID  (*FN_MMIO_WR)(DWORD val, DWORD addr);

#define VdWriteAna  ((FN_VD_WRITE_ANA)0x800BFB10)
#define CGpuWrite   ((FN_CGPU_WRITE)0x800C0190)
#define GadgetRd    ((FN_MMIO_RD)0x800C8E48)
#define GadgetWr    ((FN_MMIO_WR)0x8010B270)

#define DREAM_BEACON_MAGIC   0x53323430u /* 'S240' */
#define DREAM_BEACON_VERSION 3u
#define PLUGIN_VERSION_HEX   0x00021700u /* v2.23 */

#define DREAM_CMD_AUTO     0u
#define DREAM_CMD_240P     1u
#define DREAM_CMD_480I     2u
#define DREAM_CMD_POWEROFF 3u

#define VIDEO_MODE_480I     0
#define VIDEO_MODE_240P     1
#define VIDEO_MODE_1280X240 2
#define VIDEO_MODE_1920X240 3
#define VIDEO_MODE_2560X240 4

static DWORD mmio_read(DWORD off) {
    DWORD val;
    __sync();
    val = GadgetRd(0x7FC80000u + off);
    __emit(0x7C0006AC); /* eieio */
    return val;
}

static void mmio_write(DWORD off, DWORD val) {
    __sync();
    GadgetWr(val, 0x7FC80000u + off);
    __emit(0x7C0006AC); /* eieio */
}

typedef struct _KPATCH {
    DWORD addr;
    DWORD val;
} KPATCH;

static const KPATCH g_patches_240p[] = {
    { 0x800582C8, 0x2601820C },
    { 0x800582CC, 0x23232323 },
    { 0x8005968C, 0x83814068 },
    { 0x80059694, 0x10B420FE },
    { 0x800D04C0, 0x392A0000 },
    { 0x800CF540, 0x60000000 },
    { 0x800CF544, 0x60000000 },
};

static const KPATCH g_patches_480i[] = {
    { 0x800582C8, 0x2651B20D },
    { 0x800582CC, 0x67672323 },
    { 0x8005968C, 0x8381406A },
    { 0x80059694, 0x10C420FF },
    { 0x800D04C0, 0x392A0001 },
    { 0x800CF540, 0x81610050 },
    { 0x800CF544, 0x556B4B6C },
};

static int   g_active_mode    = VIDEO_MODE_480I;
static int   g_native_mode    = VIDEO_MODE_2560X240; /* Fixed 2560x240 Super-Resolution for native 360 & XBLA */

__declspec(noinline) static void patch_word_icache(DWORD addr, DWORD val) {
    volatile DWORD* ptr = (volatile DWORD*)addr;
    *ptr = val;
    __dcbst(0, (void*)addr);
    __sync();
    __emit(0x7C001FAC); /* icbi r0, r3 */
    __sync();
    __emit(0x4C00012C); /* isync */
}

static void apply_patches(const KPATCH* p, int count) {
    int i;
    for (i = 0; i < count; i++) {
        patch_word_icache(p[i].addr, p[i].val);
    }
}

static void hana_write_both(DWORD reg, DWORD val) {
    DWORD orig7938;
    LARGE_INTEGER_K delay5ms;
    delay5ms.QuadPart = -50000LL;
    VdWriteAna(reg, val);
    KeDelayExecutionThread(0, FALSE, &delay5ms);
    orig7938 = mmio_read(0x7938);
    mmio_write(0x7938, orig7938 & ~1u);
    mmio_write(0x7910, 1u);
    mmio_write(0x7900, 1u);
    CGpuWrite(reg, val);
    mmio_write(0x7938, orig7938);
}

static void hana_write_4(DWORD v15, DWORD v16, DWORD v26, DWORD v28) {
    DWORD orig7938;
    hana_write_both(0x15u, v15);
    hana_write_both(0x16u, v16);
    hana_write_both(0x26u, v26);
    hana_write_both(0x28u, v28);
    orig7938 = mmio_read(0x7938);
    mmio_write(0x7938, orig7938 & ~1u);
    mmio_write(0x7910, 1u);
    mmio_write(0x7900, 1u);
    CGpuWrite(0x00u, 0x0000005Eu);
    mmio_write(0x7938, orig7938);
}

static int   g_sharp_applied = 0;
static DWORD g_sharp_6584    = 0;
static DWORD g_sharp_6594    = 0;

static int is_240p_active(void) {
    return (*(volatile DWORD*)0x800582C8 == 0x2601820Cu) ? 1 : 0;
}

/* Writes Xenos D1SCL 100% pure Nearest-Neighbor (FilterType 7, unity = 512)
 * into coefficient RAM (0x6578 = bank_index, 0x657C = data port).
 *  - bank_index 0x00000000 = Vertical   (v_taps = (r6594 & 7) + 1)
 *  - bank_index 0x00020000 = Horizontal (h_taps = ((r6594 >> 8) & 0xF) + 1)
 * Must be called with D1SCL locked (0x65CC = 0x00010000) and disabled (0x6590 = 0). */
static void write_nearest_coefs(DWORD taps, DWORD bank_index) {
    int total, lo_idx, hi_idx, phase, idx;
    if (taps < 1u || taps > 10u) return;
    mmio_write(0x6578, bank_index);
    total  = 16 * (int)taps;
    lo_idx = (total - 16) / 2;
    hi_idx = (total + 16) / 2;
    for (phase = 0; phase < 9; phase++) {
        for (idx = (int)taps - 1; idx >= 0; idx -= 2) {
            int tap_lo = idx;
            int tap_hi = idx - 1;
            DWORD w_lo = (tap_lo * 16 + phase >= lo_idx && tap_lo * 16 + phase < hi_idx) ? 512u : 0u;
            DWORD w_hi = (tap_hi >= 0 && tap_hi * 16 + phase >= lo_idx && tap_hi * 16 + phase < hi_idx) ? 512u : 0u;
            DWORD hi_valid = (tap_hi >= 0) ? 0x80000000u : 0u;
            DWORD word = hi_valid | (w_hi << 19) | 0x00008000u | (w_lo << 3);
            mmio_write(0x657C, word);
        }
    }
}

/* Writes a 50/50 2-tap vertical deflicker filter (256 + 256 = 512) into Bank 0
 * when restoring 480i so interlaced dashboards/menus have zero interlace twitter. */
static void write_480i_v_deflicker_coefs(DWORD v_taps) {
    int phase, idx, center_lo, center_hi;
    if (v_taps < 2u || v_taps > 10u) return;
    center_lo = (int)v_taps / 2;
    center_hi = center_lo - 1;
    mmio_write(0x6578, 0x00000000u);
    for (phase = 0; phase < 9; phase++) {
        for (idx = (int)v_taps - 1; idx >= 0; idx -= 2) {
            int tap_lo = idx;
            int tap_hi = idx - 1;
            DWORD w_lo = (tap_lo == center_lo || tap_lo == center_hi) ? 256u : 0u;
            DWORD w_hi = (tap_hi >= 0 && (tap_hi == center_lo || tap_hi == center_hi)) ? 256u : 0u;
            DWORD hi_valid = (tap_hi >= 0) ? 0x80000000u : 0u;
            DWORD word = hi_valid | (w_hi << 19) | 0x00008000u | (w_lo << 3);
            mmio_write(0x657C, word);
        }
    }
}

/* v2.23: set every tick by WorkerThread -> 1 while a live 'S240' 240p Dream beacon exists. */
static int g_in_dream = 0;

#define V_INC_240P_MAX 0x00900000u /* 0x65C4 <= this -> native 240p source (1.0 = 0x00800000) */

/* Odd-field V_INIT (0x65DC) and Even-field V_INIT (0x65C8) for 240p anti-bounce.
 *  RUTA A (240p Dream / native 240p source): identical to v2.22 -> 0x65DC = v_init - 0x00010000.
 *  RUTA B (native 360 / XBLA, 480p-720p source):
 *    - shift = ((v_inc >> 7) + 0x800u) & ~0xFFFu (aligned to HW register step).
 *    - Floor of 0x00020000 on odd field (0x65DC) for 5-tap scaler:
 *      if v_init >= shift + 0x00020000: even = v_init, odd = v_init - shift
 *      if v_init < shift + 0x00020000:  odd = 0x00020000, even = 0x00020000 + shift. */
static void calc_v_inits(DWORD v_init_in, DWORD* p_even, DWORD* p_odd) {
    DWORD v_inc = mmio_read(0x65C4);
    if (g_in_dream || v_inc <= V_INC_240P_MAX) {
        /* RUTA A: exact v2.22 behaviour */
        *p_even = v_init_in;
        *p_odd  = (v_init_in >= 0x00010000u) ? (v_init_in - 0x00010000u) : v_init_in;
        return;
    }

    /* RUTA B: native 360 / XBLA games */
    {
        DWORD raw_shift = v_inc >> 7;
        /* Hardware registers 0x65C8 / 0x65DC mask off low fractional bits (~0xFFF). Round cleanly. */
        DWORD shift = (raw_shift + 0x800u) & ~0xFFFu;
        if (shift == 0u) shift = 0x00010000u;

        if (v_init_in >= (shift + 0x00020000u)) {
            *p_even = v_init_in;
            *p_odd  = v_init_in - shift;
        } else {
            /* 720p 16:9 / letterbox: lift both fields so 5-tap filter has safe headroom */
            *p_odd  = 0x00020000u;
            *p_even = 0x00020000u + shift;
        }
    }
}

static void sync_240p_mmio(void) {
    DWORD d, r6584, r6594, v_inc, src_w, v_taps, h_taps;
    DWORD cur_even, want_even, want_odd;
    int need_coefs;

    r6584  = mmio_read(0x6584);
    r6594  = mmio_read(0x6594);
    v_inc  = mmio_read(0x65C4);

    /* Never touch CRTC/scaler locks while the GPU is mid-reset or blanked */
    if (!g_in_dream && (r6584 == 0u || r6594 == 0u || v_inc == 0u)) {
        return;
    }

    src_w  = (r6584 >> 16) & 0xFFFFu;
    v_taps = (r6594 & 0x7u) + 1u;
    h_taps = ((r6594 >> 8) & 0xFu) + 1u;
    need_coefs = (r6594 != 0u && (!g_sharp_applied || r6584 != g_sharp_6584 || r6594 != g_sharp_6594)) ? 1 : 0;

    mmio_write(0x60E8, 1);
    mmio_write(0x65CC, 0x00010000);
    mmio_write(0x6010, 0x20B);
    mmio_write(0x6030, 1);
    mmio_write(0x6528, 0); /* D1MODE_DATA_FORMAT = 0 (Progressive, per 0x800C5DF8) */

    d = mmio_read(0x6538) & 0xFFFFu;
    mmio_write(0x6538, (d << 16) | d);

    cur_even = mmio_read(0x65C8);
    calc_v_inits(cur_even, &want_even, &want_odd);

    if (cur_even != want_even) {
        mmio_write(0x65C8, want_even);
    }
    mmio_write(0x65DC, want_odd);

    /* Only disable scaler and reload coefficient RAM when geometry/taps actually changed.
     * Avoids interrupting active D1SCL scanout during minor V_INIT/field resyncs. */
    if (need_coefs) {
        mmio_write(0x6590, 0);
        write_nearest_coefs(v_taps, 0x00000000u);
        if (src_w > 0u && src_w <= 640u) {
            write_nearest_coefs(h_taps, 0x00020000u);
        }
        mmio_write(0x6590, 1);
        g_sharp_6584    = r6584;
        g_sharp_6594    = r6594;
        g_sharp_applied = 1;
    }

    mmio_write(0x6578, 0);
    mmio_write(0x65CC, 0);
    mmio_write(0x60E8, 0);
}

static int needs_240p_mmio_sync(void) {
    DWORD r6538 = mmio_read(0x6538);
    DWORD r65c8 = mmio_read(0x65C8);
    DWORD r65dc = mmio_read(0x65DC);
    DWORD r6584 = mmio_read(0x6584);
    DWORD r6594 = mmio_read(0x6594);
    DWORD want_even, want_odd;
    if (!g_in_dream && (r6584 == 0u || r6594 == 0u || mmio_read(0x65C4) == 0u)) {
        return 0;
    }
    calc_v_inits(r65c8, &want_even, &want_odd);
    if (mmio_read(0x6010) != 0x20Bu ||
        mmio_read(0x6030) != 1u ||
        mmio_read(0x6528) != 0u ||
        (r6538 >> 16) != (r6538 & 0xFFFFu) ||
        r65c8 != want_even ||
        r65dc != want_odd ||
        !g_sharp_applied ||
        r6584 != g_sharp_6584 ||
        r6594 != g_sharp_6594 ||
        mmio_read(0x6578) != 0u) {
        return 1;
    }
    return 0;
}

static int   g_saved_480i_valid = 0;
static DWORD g_saved_6014       = 0x00260206u;
static DWORD g_saved_6018       = 0x00060000u;
static DWORD g_saved_6528       = 0u;
static DWORD g_saved_6538       = 0u;
static DWORD g_saved_6584       = 0u;
static DWORD g_saved_6594       = 0u;
static DWORD g_saved_65c8       = 0u;
static DWORD g_saved_65dc       = 0u;

static void save_480i_mmio_snapshot(void) {
    if (mmio_read(0x6010) == 0x20Cu && mmio_read(0x6110) != 0u) {
        g_saved_6014 = mmio_read(0x6014);
        g_saved_6018 = mmio_read(0x6018);
        g_saved_6528 = mmio_read(0x6528);
        g_saved_6538 = mmio_read(0x6538);
        g_saved_6584 = mmio_read(0x6584);
        g_saved_6594 = mmio_read(0x6594);
        g_saved_65c8 = mmio_read(0x65C8);
        g_saved_65dc = mmio_read(0x65DC);
        g_saved_480i_valid = 1;
    }
}

static void enable_240p_mode(int mode) {
    if (mode <= 0) mode = VIDEO_MODE_240P;

    /* Snapshot native 480i CRTC/scaler registers before switching so disable_240p() can restore them cleanly */
    if (!is_240p_active()) {
        save_480i_mmio_snapshot();
    }

    apply_patches(g_patches_240p, 7);
    hana_write_4(0x2601820Cu, 0x23232323u, 0x83814068u, 0x10B420FEu);

    g_sharp_applied = 0;
    sync_240p_mmio();
    g_active_mode = mode;
}

static void disable_240p(void) {
    DWORD even_cnt, even_v_init, odd_v_init, r6584, r6594, v_taps, cur_surf, orig7938;
    int snap_matches;
    LARGE_INTEGER_K delay20ms;
    delay20ms.QuadPart = -200000LL; /* 20ms (~1 NTSC frame) for clean interlaced sync latch */

    apply_patches(g_patches_480i, 7);
    hana_write_4(0x2651B20Du, 0x67672323u, 0x8381406Au, 0x10C420FFu);

    r6584  = mmio_read(0x6584);
    r6594  = mmio_read(0x6594);
    v_taps = (r6594 & 0x7u) + 1u;
    snap_matches = (g_saved_480i_valid && r6584 == g_saved_6584 && r6594 == g_saved_6594) ? 1 : 0;

    /* 1. Restore Xenos CRTC + D1SCL to exact 480i interlaced timings */
    mmio_write(0x60E8, 1);
    mmio_write(0x65CC, 0x00010000);
    mmio_write(0x6010, 0x20C);
    mmio_write(0x6014, g_saved_480i_valid ? g_saved_6014 : 0x00260206u);
    mmio_write(0x6018, g_saved_480i_valid ? g_saved_6018 : 0x00060000u);
    mmio_write(0x6030, 1);
    mmio_write(0x6528, g_saved_480i_valid ? g_saved_6528 : 0u);

    if (snap_matches && g_saved_6538 != 0u) {
        mmio_write(0x6538, g_saved_6538);
    } else {
        even_cnt = mmio_read(0x6538) & 0xFFFFu;
        mmio_write(0x6538, ((even_cnt + 1u) << 16) | even_cnt);
    }

    if (snap_matches && (g_saved_65c8 != 0u || g_saved_65dc != 0u)) {
        mmio_write(0x65C8, g_saved_65c8);
        mmio_write(0x65DC, g_saved_65dc);
    } else {
        even_v_init = mmio_read(0x65C8);
        odd_v_init = (even_v_init == 0x00040000u) ? 0x00060000u : (even_v_init + 0x00020000u);
        mmio_write(0x65DC, odd_v_init);
    }

    if (r6594 != 0u) {
        mmio_write(0x6590, 0);
        write_480i_v_deflicker_coefs(v_taps);
        mmio_write(0x6590, 1);
    }

    mmio_write(0x6578, 0);
    mmio_write(0x65CC, 0);
    mmio_write(0x60E8, 0);

    /* 2. Force a clean framebuffer present latch (0x6110) + HANA commit after 1 frame
     * so 480i locks cleanly without flicker or needing the Xbox Guide button. */
    KeDelayExecutionThread(0, FALSE, &delay20ms);
    cur_surf = mmio_read(0x6110);
    if (cur_surf != 0u) {
        mmio_write(0x6110, cur_surf);
    }
    orig7938 = mmio_read(0x7938);
    mmio_write(0x7938, orig7938 & ~1u);
    mmio_write(0x7910, 1u);
    mmio_write(0x7900, 1u);
    CGpuWrite(0x00u, 0x0000005Eu);
    mmio_write(0x7938, orig7938);

    g_saved_480i_valid = 0;
    g_sharp_applied = 0;
    g_active_mode = VIDEO_MODE_480I;
}

/* Forced hardware shutdown:
 *  1. Always restores kernel AV tables, scanout MMIO, and HANA to clean 480i first.
 *  2. Tries standard kernel S5 HalReturnToFirmware(5) on a helper thread (clean HDD/dash teardown).
 *  3. If S5 does not cut power within 1.5s, falls back to HalReturnToFirmware(0) & SMC 0x82 0x01 / 0x82 0x04 0x31.
 */
static DWORD WINAPI S5PowerThread(LPVOID lpParam) {
    (void)lpParam;
    HalReturnToFirmware(5);
    return 0;
}

static void force_power_off(int touch_hana) {
    unsigned char smc_cmd[16];
    HANDLE hPwr = NULL;
    ULONG tid = 0;
    LARGE_INTEGER_K delay150ms;
    LARGE_INTEGER_K delay1500ms;
    int i;
    delay150ms.QuadPart  = -1500000LL;
    delay1500ms.QuadPart = -15000000LL;

    if (is_240p_active()) {
        if (touch_hana) {
            disable_240p();
        } else {
            apply_patches(g_patches_480i, 7);
            mmio_write(0x6010, 0x20C);
        }
    }
    KeDelayExecutionThread(0, FALSE, &delay150ms);

    /* Attempt clean S5 powerdown first (now safe because 480i is restored) */
    if (ExCreateThread(&hPwr, 0, &tid, (PVOID)XapiThreadStartup, (PVOID)S5PowerThread, NULL, 2) >= 0 && hPwr != NULL) {
        NtClose(hPwr);
    }
    KeDelayExecutionThread(0, FALSE, &delay1500ms);

    /* If still powered on after 1.5s, force direct SMC cut */
    for (i = 0; i < 16; i++) {
        smc_cmd[i] = 0;
    }
    smc_cmd[0] = 0x82;
    smc_cmd[1] = 0x04;
    smc_cmd[2] = 0x31;
    smc_cmd[3] = 0x00;
    HalSendSMCMessage(smc_cmd, NULL);
    KeDelayExecutionThread(0, FALSE, &delay150ms);
    HalReturnToFirmware(0);
}

/* ==== 240p Dream Live Bidirectional IPC ('S240' v3) ==== */

typedef struct _DREAM240_IPC {
    DWORD magic;        /* 0x53323430 'S240' */
    DWORD version;      /* 3 (or 2 compat) */
    DWORD is_menu;      /* 1 = default.xex (640x480 480i), 0 = core .xex */
    DWORD width;        /* backbuffer width */
    DWORD height;       /* backbuffer height */
    DWORD in_game;      /* 1 = core .xex running */
    DWORD cmd;          /* Emulator -> Plugin: 0=auto, 1=force 240p, 2=force 480i, 3=poweroff */
    DWORD plugin_ver;   /* Plugin -> Emulator: PLUGIN_VERSION_HEX when connected */
    DWORD plugin_mode;  /* Plugin -> Emulator: active VIDEO_MODE_* */
} DREAM240_IPC;

static DWORD read_le32(const BYTE* p) {
    return (DWORD)p[0] | ((DWORD)p[1] << 8) | ((DWORD)p[2] << 16) | ((DWORD)p[3] << 24);
}

static WORD read_le16(const BYTE* p) {
    return (WORD)p[0] | ((WORD)p[1] << 8);
}

typedef struct _DREAM_REFS {
    DWORD pe_stamp;
    DWORD entry_point;
    volatile DREAM240_IPC* pBeacon;
    DWORD beacon_ver;
    int scanned;
} DREAM_REFS;

static DREAM_REFS g_dream = { 0, 0, NULL, 0, 0 };
static int g_scan_retries = 0;

static void scan_dream_binary(void) {
    const BYTE* base = (const BYTE*)0x82000000u;
    DWORD e_lfanew, pe_stamp, entry_point, opt_size, num_sec;
    DWORD data_start = 0, data_end = 0;
    const BYTE* nt;
    const BYTE* sec;
    DWORD i, addr;

    if (!MmIsAddressValid((PVOID)base) || base[0] != 'M' || base[1] != 'Z') {
        g_dream.scanned = 0;
        g_dream.pBeacon = NULL;
        g_dream.beacon_ver = 0;
        g_scan_retries = 0;
        return;
    }
    e_lfanew = read_le32(base + 0x3C);
    if (e_lfanew < 0x40u || e_lfanew > 0x400u) return;
    nt = base + e_lfanew;
    if (!MmIsAddressValid((PVOID)nt) || nt[0] != 'P' || nt[1] != 'E') return;

    pe_stamp = read_le32(nt + 8);
    num_sec = read_le16(nt + 6);
    opt_size = read_le16(nt + 20);
    entry_point = read_le32(nt + 40);

    if (g_dream.scanned && g_dream.pe_stamp == pe_stamp && g_dream.entry_point == entry_point) {
        if (g_dream.pBeacon != NULL) {
            if (MmIsAddressValid((PVOID)g_dream.pBeacon) &&
                g_dream.pBeacon->magic == DREAM_BEACON_MAGIC) {
                return;
            }
            g_dream.pBeacon = NULL;
            g_dream.beacon_ver = 0;
            g_scan_retries = 0;
        } else if (g_scan_retries >= 60) {
            return;
        }
        g_scan_retries++;
    } else {
        g_scan_retries = 0;
    }

    g_dream.pe_stamp = pe_stamp;
    g_dream.entry_point = entry_point;
    g_dream.pBeacon = NULL;
    g_dream.beacon_ver = 0;
    g_dream.scanned = 1;

    sec = nt + 24 + opt_size;
    for (i = 0; i < num_sec && i < 16; i++) {
        const BYTE* sh = sec + i * 40u;
        DWORD vsize, vaddr;
        if (!MmIsAddressValid((PVOID)sh)) break;
        vsize = read_le32(sh + 8);
        vaddr = 0x82000000u + read_le32(sh + 12);
        if (sh[0] == '.' && sh[1] == 'd' && sh[2] == 'a' && sh[3] == 't') {
            data_start = vaddr;
            data_end = vaddr + vsize;
        }
    }

    if (data_start >= 0x82010000u && data_end > data_start + 64u) {
        for (addr = data_start; addr < data_end - sizeof(DREAM240_IPC); addr += 4u) {
            DWORD ver;
            if ((addr & 0xFFFu) == 0u || addr == data_start) {
                if (!MmIsAddressValid((PVOID)addr)) {
                    addr = (addr & ~0xFFFu) + 0xFFCu;
                    continue;
                }
            }
            if (*(const DWORD*)addr == DREAM_BEACON_MAGIC) {
                ver = *(const DWORD*)(addr + 4u);
                if (ver == DREAM_BEACON_VERSION || ver == 2u) {
                    g_dream.pBeacon = (volatile DREAM240_IPC*)addr;
                    g_dream.beacon_ver = ver;
                    break;
                }
            }
        }
    }
}

static volatile DWORD g_diag[4] = { 0x5432344Fu, 0, 0, 0 }; /* "T24O", pXamInputGetState, last_rc_btn, toggles */

typedef struct _T240_LIVE {
    DWORD magic;       /* 0x544C4956 'TLIV' */
    DWORD cmd;         /* 0=idle, 1=apply live params, 2=clear override */
    DWORD h15;
    DWORD h16;
    DWORD h26;
    DWORD h28;
    DWORD r6010;
    DWORD r6014;
    DWORD r6018;
    DWORD r6030;
    DWORD r6528;       /* 0xFFFFFFFF -> untouched */
    DWORD r6538;       /* 0 -> auto (d<<16)|d */
    DWORD r65dc;       /* 0xFFFFFFFF -> auto r65c8 */
    DWORD mmio_6010;
    DWORD mmio_6014;
    DWORD mmio_6018;
    DWORD mmio_6030;
    DWORD mmio_6528;
    DWORD mmio_6538;
    DWORD mmio_6584;
    DWORD mmio_6594;
    DWORD mmio_65c4;
    DWORD mmio_65c8;
    DWORD mmio_65dc;
} T240_LIVE;

static volatile T240_LIVE g_live = {
    0x544C4956u, 0u,
    0x2601820Cu, 0x00000000u, 0x83814068u, 0x10B420FEu,
    0x20Bu, 0u, 0u, 1u, 0u, 0u, 0xFFFFFFFFu,
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u
};
static int g_live_override = 0;

static void update_live_telemetry(void) {
    g_live.mmio_6010 = mmio_read(0x6010);
    g_live.mmio_6014 = mmio_read(0x6014);
    g_live.mmio_6018 = mmio_read(0x6018);
    g_live.mmio_6030 = mmio_read(0x6030);
    g_live.mmio_6528 = mmio_read(0x6528);
    g_live.mmio_6538 = mmio_read(0x6538);
    g_live.mmio_6584 = mmio_read(0x6584);
    g_live.mmio_6594 = mmio_read(0x6594);
    g_live.mmio_65c4 = mmio_read(0x65C4);
    g_live.mmio_65c8 = mmio_read(0x65C8);
    g_live.mmio_65dc = mmio_read(0x65DC);
}

static void process_live_cmd(void) {
    DWORD cmd = g_live.cmd;
    if (cmd == 2u) {
        g_live.cmd = 0u;
        g_live_override = 0;
        sync_240p_mmio();
        update_live_telemetry();
        return;
    }
    if (cmd == 1u) {
        DWORD d, v_init, live_even, live_odd;
        g_live.cmd = 0u;
        g_live_override = 1;
        hana_write_4(g_live.h15, g_live.h16, g_live.h26, g_live.h28);
        mmio_write(0x60E8, 1);
        mmio_write(0x65CC, 0x00010000);
        mmio_write(0x6010, g_live.r6010);
        if (g_live.r6014 != 0u) mmio_write(0x6014, g_live.r6014);
        if (g_live.r6018 != 0u) mmio_write(0x6018, g_live.r6018);
        mmio_write(0x6030, g_live.r6030);
        if (g_live.r6528 != 0xFFFFFFFFu) mmio_write(0x6528, g_live.r6528);
        d = mmio_read(0x6538) & 0xFFFFu;
        mmio_write(0x6538, (g_live.r6538 != 0u) ? g_live.r6538 : ((d << 16) | d));
        v_init = mmio_read(0x65C8);
        calc_v_inits(v_init, &live_even, &live_odd);
        if (g_live.r65dc != 0xFFFFFFFFu) {
            mmio_write(0x65DC, g_live.r65dc);
        } else {
            if (v_init != live_even) mmio_write(0x65C8, live_even);
            mmio_write(0x65DC, live_odd);
        }
        mmio_write(0x6578, 0);
        mmio_write(0x65CC, 0);
        mmio_write(0x60E8, 0);
        update_live_telemetry();
    }
}

#define NATIVE_STATE_OFF        0
#define NATIVE_STATE_BOOT_WAIT  1
#define NATIVE_STATE_ACTIVE     2
#define NATIVE_STATE_RECOVERING 3

static DWORD WINAPI WorkerThread(LPVOID lpParam) {
    LARGE_INTEGER_K delay50ms;
    LARGE_INTEGER_K delay17ms;
    LARGE_INTEGER_K delay2s;
    FN_XAM_INPUT_GET_STATE pXamInputGetState = NULL;
    DWORD init_5b4;
    int combo_240_held = 0;
    int kernel_pwr_ticks = 0;
    int manual_override = 0;
    int dream_owned = 0;
    int dream_wait_ticks = 0;
    int native_state = NATIVE_STATE_OFF;
    int title_has_flipped = 0;
    DWORD last_surf = 0;
    int boot_ticks = 0;
    int boot_flips = 0;
    int boot_stable_ticks = 0;
    int stall_ticks = 0;
    int blank_ticks = 0;
    int rec_ticks = 0;
    int rec_flips = 0;
    int rec_burst_count = 0;
    int rec_burst_window = 0;
    int sync_debounce = 0;
    DWORD deb_6010 = 0;
    DWORD deb_6584 = 0;
    DWORD deb_6594 = 0;
    DWORD deb_65c4 = 0;
    DWORD last_beacon_stamp = 0;
    DWORD last_beacon_ep = 0;
    DWORD tick = 0;

    delay50ms.QuadPart = -500000LL;
    delay17ms.QuadPart = -170000LL;
    delay2s.QuadPart = -20000000LL;
    (void)lpParam;

    /* Wait briefly at DashLaunch boot */
    KeDelayExecutionThread(0, FALSE, &delay2s);

    init_5b4 = *(volatile DWORD*)0x801705B4u;

    for (;;) {
        XINPUT_STATE xs;

        KeDelayExecutionThread(0, FALSE, &delay50ms);
        tick++;

        if (rec_burst_window > 0) {
            rec_burst_window--;
            if (rec_burst_window == 0) {
                rec_burst_count = 0;
            }
        }

        process_live_cmd();
        if ((tick % 10u) == 0u && mmio_read(0x6110) != 0u) {
            update_live_telemetry();
        }

        /* Kernel shutdown watchdog (physical Power button / Guide -> Turn Off):
         * Restore 480i immediately when kernel DPC 0x801705B4 fires shutdown, WITHOUT blocking the loop. */
        if ((*(volatile DWORD*)0x801705B4u != init_5b4)) {
            if (is_240p_active()) {
                disable_240p();
            }
            kernel_pwr_ticks++;
            if (kernel_pwr_ticks == 30) {
                force_power_off(0);
            }
        } else {
            kernel_pwr_ticks = 0;
        }

        /* Dynamically resolve XamInputGetState (zero static xam.xex imports) */
        if (pXamInputGetState == NULL) {
            HANDLE hXam = NULL;
            if (XexGetModuleHandle("xam.xex", &hXam) >= 0 && hXam != NULL) {
                PVOID proc = NULL;
                if (XexGetProcedureAddress(hXam, 401, &proc) >= 0 && proc != NULL) {
                    pXamInputGetState = (FN_XAM_INPUT_GET_STATE)proc;
                    g_diag[1] = (DWORD)proc;
                }
            }
        }

        /* Controller shortcut:
         *  - LT + RT + L3 + R3 (simultaneous) -> Native 360/XBLA toggle 2560x240 <-> 480i (outside 240p Dream)
         */
        if (pXamInputGetState != NULL) {
            DWORD u;
            int now_240 = 0;
            WORD sticks = XINPUT_GAMEPAD_LEFT_THUMB | XINPUT_GAMEPAD_RIGHT_THUMB;
            for (u = 0; u < 4; u++) {
                DWORD rc = pXamInputGetState(u, 0, &xs);
                if (rc != 0) {
                    rc = pXamInputGetState(u, 0x000000FFu, &xs);
                }
                if (u == 0) {
                    g_diag[2] = (rc == 0)
                        ? (((DWORD)xs.Gamepad.bLeftTrigger << 24) | ((DWORD)xs.Gamepad.bRightTrigger << 16) | (DWORD)xs.Gamepad.wButtons)
                        : (0xE0000000u | (rc & 0xFFFFu));
                }
                if (rc == 0 && (xs.Gamepad.bLeftTrigger > 100) && (xs.Gamepad.bRightTrigger > 100) &&
                    ((xs.Gamepad.wButtons & sticks) == sticks)) {
                    now_240 = 1;
                    break;
                }
            }

            if (now_240 && !combo_240_held) {
                combo_240_held = 1;
                if (g_dream.pBeacon == NULL) {
                    g_diag[3]++;
                    dream_owned = 0;
                    if (is_240p_active() || native_state != NATIVE_STATE_OFF) {
                        manual_override = 0;
                        native_state = NATIVE_STATE_OFF;
                        g_live_override = 0;
                        disable_240p();
                    } else {
                        manual_override = 1;
                        native_state = NATIVE_STATE_ACTIVE;
                        g_live_override = 0;
                        stall_ticks = 0;
                        blank_ticks = 0;
                        sync_debounce = 0;
                        enable_240p_mode(g_native_mode);
                    }
                }
            } else if (!now_240) {
                combo_240_held = 0;
            }
        }

        /* Direct 240p Dream Emulator Control ('S240' v3 IPC) & Native 360/XBLA Title Persistence */
        scan_dream_binary();
        g_in_dream = (g_dream.pBeacon != NULL && MmIsAddressValid((PVOID)g_dream.pBeacon) &&
                      g_dream.pBeacon->magic == DREAM_BEACON_MAGIC) ? 1 : 0;

        /* If title image is momentarily unmapped during XamLoaderLaunchTitle, disarm 240p immediately */
        if (!g_dream.scanned) {
            if (!g_in_dream && is_240p_active()) {
                disable_240p();
            }
            if (manual_override) {
                native_state = NATIVE_STATE_BOOT_WAIT;
                boot_ticks = 0;
                boot_flips = 0;
                boot_stable_ticks = 0;
                title_has_flipped = 0;
                last_surf = 0;
            }
            continue;
        }

        if (g_dream.scanned &&
            (g_dream.pe_stamp != last_beacon_stamp || g_dream.entry_point != last_beacon_ep)) {
            last_beacon_stamp = g_dream.pe_stamp;
            last_beacon_ep = g_dream.entry_point;
            dream_wait_ticks = 0;
            g_saved_480i_valid = 0;
            g_sharp_applied = 0;
            title_has_flipped = 0;
            last_surf = 0;
            boot_ticks = 0;
            boot_flips = 0;
            boot_stable_ticks = 0;
            stall_ticks = 0;
            blank_ticks = 0;
            rec_ticks = 0;
            rec_flips = 0;
            sync_debounce = 0;
            rec_burst_count = 0;
            rec_burst_window = 0;

            /* On any non-Dream title launch: ALWAYS drop to stock 480i first so D3D9 / VdSetDisplayMode
             * initializes without freezing on the initial loading screen. */
            if (!g_in_dream) {
                if (is_240p_active()) {
                    disable_240p();
                }
                native_state = manual_override ? NATIVE_STATE_BOOT_WAIT : NATIVE_STATE_OFF;
            }
        }

        if (g_dream.pBeacon != NULL && MmIsAddressValid((PVOID)g_dream.pBeacon) &&
            g_dream.pBeacon->magic == DREAM_BEACON_MAGIC) {
            DWORD b_menu = g_dream.pBeacon->is_menu;
            DWORD b_w = g_dream.pBeacon->width;
            DWORD b_h = g_dream.pBeacon->height;
            DWORD b_cmd = (g_dream.beacon_ver >= 3u) ? g_dream.pBeacon->cmd : DREAM_CMD_AUTO;

            /* Publish live plugin status back to 240p Dream memory */
            if (g_dream.beacon_ver >= 3u) {
                g_dream.pBeacon->plugin_ver = PLUGIN_VERSION_HEX;
                g_dream.pBeacon->plugin_mode = (DWORD)(is_240p_active() ? g_active_mode : VIDEO_MODE_480I);
            }

            if (b_cmd == DREAM_CMD_POWEROFF) {
                g_dream.pBeacon->cmd = DREAM_CMD_AUTO;
                force_power_off(1);
            } else if (b_cmd == DREAM_CMD_480I) {
                dream_owned = 0;
                manual_override = 0;
                native_state = NATIVE_STATE_OFF;
                if (is_240p_active()) {
                    disable_240p();
                }
            } else if (b_cmd == DREAM_CMD_240P && b_w > 0u) {
                int res_mode = VIDEO_MODE_240P;
                if (b_w >= 1900u) res_mode = VIDEO_MODE_1920X240;
                else if (b_w >= 1200u) res_mode = VIDEO_MODE_1280X240;
                dream_owned = 1;
                manual_override = 0;
                native_state = NATIVE_STATE_OFF;
                if (!is_240p_active() || g_active_mode != res_mode) {
                    g_live_override = 0;
                    enable_240p_mode(res_mode);
                } else if (!g_live_override && needs_240p_mmio_sync()) {
                    sync_240p_mmio();
                }
            } else if (b_w > 0u && b_h > 0u) {
                manual_override = 0;
                native_state = NATIVE_STATE_OFF;
                if (b_menu != 0u || b_h > 256u) {
                    dream_owned = 0;
                    if (is_240p_active()) {
                        disable_240p();
                    }
                } else {
                    int res_mode = VIDEO_MODE_240P;
                    if (b_w >= 1900u) res_mode = VIDEO_MODE_1920X240;
                    else if (b_w >= 1200u) res_mode = VIDEO_MODE_1280X240;
                    dream_owned = 1;

                    if (!is_240p_active() || g_active_mode != res_mode) {
                        if (mmio_read(0x6110) != 0u || dream_wait_ticks >= 20) {
                            g_live_override = 0;
                            enable_240p_mode(res_mode);
                        } else {
                            dream_wait_ticks++;
                        }
                    } else if (!g_live_override && needs_240p_mmio_sync()) {
                        sync_240p_mmio();
                    }
                }
            }

            if (g_dream.beacon_ver >= 3u) {
                g_dream.pBeacon->plugin_mode = (DWORD)(is_240p_active() ? g_active_mode : VIDEO_MODE_480I);
            }
            continue;
        } else {
            int want_native_240p = manual_override ? 1 : 0;
            dream_wait_ticks = 0;
            if (dream_owned) {
                dream_owned = 0;
                if (!want_native_240p && is_240p_active()) {
                    disable_240p();
                }
            }

            if (!want_native_240p) {
                native_state = NATIVE_STATE_OFF;
                if (is_240p_active()) {
                    disable_240p();
                }
            } else {
                DWORD cur_surf = mmio_read(0x6110);
                DWORD r6584 = mmio_read(0x6584);
                DWORD r6594 = mmio_read(0x6594);
                DWORD r65c4 = mmio_read(0x65C4);
                int scanout_ready = (cur_surf != 0u && r6584 != 0u && r6594 != 0u && r65c4 != 0u) ? 1 : 0;

                if (native_state == NATIVE_STATE_OFF) {
                    native_state = NATIVE_STATE_BOOT_WAIT;
                    boot_ticks = 0;
                    boot_flips = 0;
                    boot_stable_ticks = 0;
                }

                if (native_state == NATIVE_STATE_BOOT_WAIT) {
                    /* Keep stock 480i until the new title finishes D3D init and actively presents frames */
                    if (is_240p_active()) {
                        disable_240p();
                    }
                    boot_ticks++;
                    if (scanout_ready) {
                        boot_stable_ticks++;
                        if (last_surf != 0u && cur_surf != last_surf) {
                            boot_flips++;
                            title_has_flipped = 1;
                        }
                        last_surf = cur_surf;
                        if ((boot_ticks >= 50 && boot_flips >= 10) ||
                            (boot_ticks >= 160 && boot_stable_ticks >= 20)) {
                            enable_240p_mode(g_native_mode);
                            native_state = NATIVE_STATE_ACTIVE;
                            stall_ticks = 0;
                            blank_ticks = 0;
                            sync_debounce = 0;
                        }
                    } else {
                        boot_stable_ticks = 0;
                        last_surf = 0;
                    }
                } else if (native_state == NATIVE_STATE_RECOVERING) {
                    /* Temporarily in 480i to release a VBlank/transition stall; re-engage 240p once healthy flips resume */
                    if (is_240p_active()) {
                        disable_240p();
                    }
                    rec_ticks++;
                    if (scanout_ready) {
                        if (last_surf != 0u && cur_surf != last_surf) {
                            rec_flips++;
                            title_has_flipped = 1;
                        }
                        last_surf = cur_surf;
                        if (rec_flips >= 8 && rec_ticks >= 12) {
                            enable_240p_mode(g_native_mode);
                            native_state = NATIVE_STATE_ACTIVE;
                            stall_ticks = 0;
                            blank_ticks = 0;
                            sync_debounce = 0;
                        }
                    } else {
                        last_surf = 0;
                    }
                } else {
                    /* NATIVE_STATE_ACTIVE: 2560x240 engaged with transition stabilization + stall auto-recovery */
                    if (cur_surf == 0u) {
                        blank_ticks++;
                        sync_debounce = 0;
                        if (blank_ticks >= 10) {
                            /* Blanked >= 500ms (display reset / scene load): drop to 480i until flips resume */
                            if (is_240p_active()) {
                                disable_240p();
                            }
                            native_state = NATIVE_STATE_RECOVERING;
                            rec_ticks = 0;
                            rec_flips = 0;
                            blank_ticks = 0;
                            stall_ticks = 0;
                            last_surf = 0;
                        }
                    } else {
                        blank_ticks = 0;
                        if (last_surf != 0u && cur_surf != last_surf) {
                            title_has_flipped = 1;
                            stall_ticks = 0;
                        } else if (title_has_flipped) {
                            stall_ticks++;
                            /* Anti-aliasing 17ms double-check (~1 NTSC frame) before declaring a stall */
                            if (stall_ticks >= 20) {
                                DWORD peek_surf;
                                KeDelayExecutionThread(0, FALSE, &delay17ms);
                                peek_surf = mmio_read(0x6110);
                                if (peek_surf != 0u && peek_surf != cur_surf) {
                                    cur_surf = peek_surf;
                                    stall_ticks = 0;
                                }
                            }
                            {
                                int stall_thresh = (rec_burst_count >= 3) ? 60 : 30;
                                if (stall_ticks >= stall_thresh) {
                                    /* Confirmed presentation stall: auto-restore 480i in background to unstick game */
                                    if (is_240p_active()) {
                                        disable_240p();
                                    }
                                    native_state = NATIVE_STATE_RECOVERING;
                                    rec_ticks = 0;
                                    rec_flips = 0;
                                    stall_ticks = 0;
                                    sync_debounce = 0;
                                    rec_burst_count++;
                                    rec_burst_window = 300;
                                }
                            }
                        }
                        last_surf = cur_surf;

                        /* Debounced MMIO/HANA sync while active (never race an in-flight VdSetDisplayMode) */
                        if (native_state == NATIVE_STATE_ACTIVE && scanout_ready) {
                            if (!is_240p_active()) {
                                enable_240p_mode(g_native_mode);
                                sync_debounce = 0;
                            } else if (needs_240p_mmio_sync()) {
                                DWORD r6010 = mmio_read(0x6010);
                                if (sync_debounce == 0 ||
                                    r6010 != deb_6010 || r6584 != deb_6584 ||
                                    r6594 != deb_6594 || r65c4 != deb_65c4) {
                                    deb_6010 = r6010;
                                    deb_6584 = r6584;
                                    deb_6594 = r6594;
                                    deb_65c4 = r65c4;
                                    sync_debounce = 1;
                                } else {
                                    sync_debounce++;
                                    if (sync_debounce >= 2) {
                                        if (r6010 != 0x20Bu) {
                                            hana_write_4(0x2601820Cu, 0x23232323u, 0x83814068u, 0x10B420FEu);
                                        }
                                        sync_240p_mmio();
                                        sync_debounce = 0;
                                    }
                                }
                            } else {
                                sync_debounce = 0;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    (void)hModule;
    (void)lpReserved;
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        HANDLE hThread = NULL;
        ULONG tid = 0;
        NTSTATUS st = ExCreateThread(&hThread, 0, &tid, (PVOID)XapiThreadStartup, (PVOID)WorkerThread, NULL, 2);
        if (st >= 0 && hThread != NULL) {
            PVOID pThread = NULL;
            if (ObReferenceObjectByHandle(hThread, NULL, &pThread) >= 0 && pThread != NULL) {
                KeSetPriorityThread(pThread, 20); /* Beat Genesis-Plus-GX prio 17 on proc 2 */
                ObDereferenceObject(pThread);
            }
            NtClose(hThread);
        }
    }
    return TRUE;
}
