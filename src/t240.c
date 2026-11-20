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

static const char* CMD_PATH    = "\\Device\\Harddisk0\\Partition1\\AuroraLaunch\\User\\Scripts\\Utility\\Toggle_240p\\cmd.txt";
static const char* STATE_PATH  = "\\Device\\Harddisk0\\Partition1\\AuroraLaunch\\User\\Scripts\\Utility\\Toggle_240p\\state.txt";
static const char* PRESET_PATH = "\\Device\\Harddisk0\\Partition1\\AuroraLaunch\\User\\Scripts\\Utility\\Toggle_240p\\preset.txt";

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
static int   g_script_preset  = VIDEO_MODE_240P;

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
    DWORD d, r6584, r6594, src_w, v_taps, h_taps;
    DWORD cur_even, want_even, want_odd;
    r6584  = mmio_read(0x6584);
    r6594  = mmio_read(0x6594);
    src_w  = (r6584 >> 16) & 0xFFFFu;
    v_taps = (r6594 & 0x7u) + 1u;
    h_taps = ((r6594 >> 8) & 0xFu) + 1u;

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

    if (r6594 != 0u) {
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

static void enable_240p_mode(int mode) {
    if (mode <= 0) mode = VIDEO_MODE_240P;

    apply_patches(g_patches_240p, 7);
    hana_write_4(0x2601820Cu, 0x23232323u, 0x83814068u, 0x10B420FEu);

    g_sharp_applied = 0;
    sync_240p_mmio();
    g_active_mode = mode;
}

static void disable_240p(void) {
    DWORD even_cnt, even_v_init, odd_v_init, r6594, v_taps;
    apply_patches(g_patches_480i, 7);
    hana_write_4(0x2651B20Du, 0x67672323u, 0x8381406Au, 0x10C420FFu);

    r6594  = mmio_read(0x6594);
    v_taps = (r6594 & 0x7u) + 1u;

    mmio_write(0x60E8, 1);
    mmio_write(0x65CC, 0x00010000);
    mmio_write(0x6010, 0x20C);
    mmio_write(0x6014, 0x00260206);
    mmio_write(0x6018, 0x00060000);
    mmio_write(0x6030, 1);
    mmio_write(0x6528, 0);

    even_cnt = mmio_read(0x6538) & 0xFFFFu;
    mmio_write(0x6538, ((even_cnt + 1u) << 16) | even_cnt);

    even_v_init = mmio_read(0x65C8);
    odd_v_init = (even_v_init == 0x00040000u) ? 0x00060000u : (even_v_init + 0x00020000u);
    mmio_write(0x65DC, odd_v_init);

    if (r6594 != 0u) {
        mmio_write(0x6590, 0);
        write_480i_v_deflicker_coefs(v_taps);
        mmio_write(0x6590, 1);
    }

    mmio_write(0x6578, 0);
    mmio_write(0x65CC, 0);
    mmio_write(0x60E8, 0);
    g_sharp_applied = 0;
    g_active_mode = VIDEO_MODE_480I;
}

static void write_state_file(void) {
    ANSI_STRING as;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK iosb;
    HANDLE hFile = NULL;
    NTSTATUS st;
    LARGE_INTEGER_K zero;
    const char* txt;
    ULONG len;

    zero.QuadPart = 0;
    if (!is_240p_active()) {
        txt = "480i";
        len = 4;
    } else if (g_active_mode == VIDEO_MODE_1280X240) {
        txt = "1280x240p";
        len = 9;
    } else if (g_active_mode == VIDEO_MODE_1920X240) {
        txt = "1920x240p";
        len = 9;
    } else {
        txt = "320x240p";
        len = 8;
    }

    RtlInitAnsiString(&as, STATE_PATH);
    oa.RootDirectory = NULL;
    oa.ObjectName = &as;
    oa.Attributes = 0x40; /* OBJ_CASE_INSENSITIVE */

    st = NtCreateFile(&hFile, 0x40100000, &oa, &iosb,
                      NULL, 0x80, 7, 5 /* FILE_OVERWRITE_IF */, 0x20);
    if (st >= 0 && hFile != NULL) {
        NtWriteFile(hFile, NULL, NULL, NULL, &iosb, (PVOID)txt, len, &zero);
        NtClose(hFile);
    }
}

static void read_script_preset(void) {
    ANSI_STRING as;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK iosb;
    HANDLE hFile = NULL;
    NTSTATUS st;
    LARGE_INTEGER_K zero;
    char buf[16];
    zero.QuadPart = 0;

    RtlInitAnsiString(&as, PRESET_PATH);
    oa.RootDirectory = NULL;
    oa.ObjectName = &as;
    oa.Attributes = 0x40;

    st = NtOpenFile(&hFile, 0x80100000, &oa, &iosb, 7, 0x20);
    if (st < 0 || hFile == NULL) {
        return;
    }
    buf[0] = 0;
    st = NtReadFile(hFile, NULL, NULL, NULL, &iosb, buf, sizeof(buf) - 1, &zero);
    NtClose(hFile);
    if (st >= 0 && iosb.Information > 0) {
        if (buf[0] == '3') g_script_preset = VIDEO_MODE_1280X240;
        else if (buf[0] == '4') g_script_preset = VIDEO_MODE_1920X240;
        else if (buf[0] == '1') g_script_preset = VIDEO_MODE_240P;
    }
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

static DWORD WINAPI WorkerThread(LPVOID lpParam) {
    LARGE_INTEGER_K delay50ms;
    LARGE_INTEGER_K delay2s;
    FN_XAM_INPUT_GET_STATE pXamInputGetState = NULL;
    DWORD init_5b4;
    int combo_240_held = 0;
    int kernel_pwr_ticks = 0;
    int manual_override = 0;
    int dream_owned = 0;
    int dream_wait_ticks = 0;
    int title_boot_ticks = 0;
    DWORD last_beacon_stamp = 0;
    DWORD last_beacon_ep = 0;
    DWORD tick = 0;

    delay50ms.QuadPart = -500000LL;
    delay2s.QuadPart = -20000000LL;
    (void)lpParam;

    /* Wait briefly for HDD mount at DashLaunch boot */
    KeDelayExecutionThread(0, FALSE, &delay2s);

    init_5b4  = *(volatile DWORD*)0x801705B4u;

    read_script_preset();
    write_state_file();

    for (;;) {
        ANSI_STRING as;
        OBJECT_ATTRIBUTES oa;
        IO_STATUS_BLOCK iosb;
        HANDLE hFile = NULL;
        NTSTATUS st;
        char buf[8];
        LARGE_INTEGER_K zero;
        XINPUT_STATE xs;
        zero.QuadPart = 0;

        KeDelayExecutionThread(0, FALSE, &delay50ms);
        tick++;

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
         *  - LT + RT + L3 + R3 (simultaneous) -> Native 360/Arcade toggle 240p <-> 480i (when NOT in 240p Dream)
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

            /* Independent Aurora-preset controller toggle (ONLY active outside 240p Dream so script/native never collide with emulator) */
            if (now_240 && !combo_240_held) {
                combo_240_held = 1;
                if (g_dream.pBeacon == NULL) {
                    g_diag[3]++;
                    dream_owned = 0;
                    if (title_boot_ticks == 0 && mmio_read(0x6110) != 0u) {
                        read_script_preset();
                    }
                    if (is_240p_active()) {
                        manual_override = 0;
                        disable_240p();
                    } else {
                        manual_override = 1;
                        enable_240p_mode(g_script_preset);
                    }
                    if (title_boot_ticks == 0 && mmio_read(0x6110) != 0u) {
                        write_state_file();
                    }
                }
            } else if (!now_240) {
                combo_240_held = 0;
            }
        }

        /* Direct 240p Dream Emulator Control ('S240' v3 IPC) & Native 360/Arcade Title Persistence */
        scan_dream_binary();
        g_in_dream = (g_dream.pBeacon != NULL && MmIsAddressValid((PVOID)g_dream.pBeacon) &&
                      g_dream.pBeacon->magic == DREAM_BEACON_MAGIC) ? 1 : 0;
        if (g_dream.scanned &&
            (g_dream.pe_stamp != last_beacon_stamp || g_dream.entry_point != last_beacon_ep)) {
            last_beacon_stamp = g_dream.pe_stamp;
            last_beacon_ep = g_dream.entry_point;
            dream_wait_ticks = 0;
            title_boot_ticks = 40; /* 2.0s zero-HDD-IO grace window during title transition */
            g_sharp_applied = 0;
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
                if (is_240p_active()) {
                    disable_240p();
                }
            } else if (b_cmd == DREAM_CMD_240P && b_w > 0u) {
                int res_mode = VIDEO_MODE_240P;
                if (b_w >= 1900u) res_mode = VIDEO_MODE_1920X240;
                else if (b_w >= 1200u) res_mode = VIDEO_MODE_1280X240;
                dream_owned = 1;
                manual_override = 0;
                if (!is_240p_active() || g_active_mode != res_mode) {
                    g_live_override = 0;
                    enable_240p_mode(res_mode);
                } else if (!g_live_override && needs_240p_mmio_sync()) {
                    sync_240p_mmio();
                }
            } else if (b_w > 0u && b_h > 0u) {
                manual_override = 0;
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
            dream_wait_ticks = 0;
            if (dream_owned) {
                dream_owned = 0;
                if (!manual_override && is_240p_active()) {
                    disable_240p();
                }
            } else if (manual_override) {
                /* Native Xbox 360 / XBLA Arcade game or Aurora with 240p enabled manually:
                 * Keep 240p active across title launches and re-sync Xenos/HANA once scanout is live. */
                if (mmio_read(0x6110) != 0u) {
                    if (!is_240p_active()) {
                        enable_240p_mode(g_script_preset);
                    } else if (needs_240p_mmio_sync()) {
                        if (mmio_read(0x6010) != 0x20Bu) {
                            hana_write_4(0x2601820Cu, 0x23232323u, 0x83814068u, 0x10B420FEu);
                        }
                        sync_240p_mmio();
                    }
                }
            } else if (is_240p_active()) {
                disable_240p();
            }
        }

        /* Skip HDD polling during title transitions or while scanout is blank to avoid FATX contention */
        if (title_boot_ticks > 0) {
            title_boot_ticks--;
            continue;
        }
        if (mmio_read(0x6110) == 0u) {
            continue;
        }

        /* Poll Aurora script cmd.txt every 100ms */
        if ((tick & 1u) != 0) {
            continue;
        }

        RtlInitAnsiString(&as, CMD_PATH);
        oa.RootDirectory = NULL;
        oa.ObjectName = &as;
        oa.Attributes = 0x40;

        st = NtOpenFile(&hFile, 0x80100000, &oa, &iosb, 7, 0x20);
        if (st < 0 || hFile == NULL) {
            continue;
        }

        buf[0] = 0;
        st = NtReadFile(hFile, NULL, NULL, NULL, &iosb, buf, 4, &zero);
        NtClose(hFile);
        if (st < 0 || iosb.Information == 0) {
            KeDelayExecutionThread(0, FALSE, &delay50ms);
            continue;
        }

        hFile = NULL;
        st = NtOpenFile(&hFile, 0x80110000, &oa, &iosb, 7, 0x1020);
        if (st >= 0 && hFile != NULL) {
            NtClose(hFile);
        }

        if (buf[0] == 'r') {
            /* Aurora script armed a resolution preset without activating 240p in Aurora */
            read_script_preset();
            write_state_file();
        } else if (buf[0] == 'p') {
            force_power_off(1);
        } else if (g_dream.pBeacon == NULL) {
            dream_owned = 0;
            if (buf[0] == '0') {
                manual_override = 0;
                disable_240p();
                write_state_file();
            } else if (buf[0] == '1') {
                manual_override = 1;
                g_script_preset = VIDEO_MODE_240P;
                enable_240p_mode(VIDEO_MODE_240P);
                write_state_file();
            } else if (buf[0] == '3') {
                manual_override = 1;
                g_script_preset = VIDEO_MODE_1280X240;
                enable_240p_mode(VIDEO_MODE_1280X240);
                write_state_file();
            } else if (buf[0] == '4') {
                manual_override = 1;
                g_script_preset = VIDEO_MODE_1920X240;
                enable_240p_mode(VIDEO_MODE_1920X240);
                write_state_file();
            } else if (buf[0] == 't') {
                read_script_preset();
                if (is_240p_active()) {
                    manual_override = 0;
                    disable_240p();
                } else {
                    manual_override = 1;
                    enable_240p_mode(g_script_preset);
                }
                write_state_file();
            } else if (buf[0] == 's') {
                write_state_file();
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
