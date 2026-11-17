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
#ifdef __cplusplus
}
#endif

typedef DWORD (*FN_XAM_INPUT_GET_STATE)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_STATE* pState);

typedef DWORD (*FN_VD_READ_ANA)(DWORD reg);
typedef VOID  (*FN_VD_WRITE_ANA)(DWORD reg, DWORD val);
typedef VOID  (*FN_CGPU_WRITE)(DWORD reg, DWORD val);
typedef DWORD (*FN_MMIO_RD)(DWORD addr);
typedef VOID  (*FN_MMIO_WR)(DWORD val, DWORD addr);

#define VdReadAna   ((FN_VD_READ_ANA)0x800BFA50)
#define VdWriteAna  ((FN_VD_WRITE_ANA)0x800BFB10)
#define CGpuWrite   ((FN_CGPU_WRITE)0x800C0190)
#define GadgetRd    ((FN_MMIO_RD)0x800C8E48)
#define GadgetWr    ((FN_MMIO_WR)0x8010B270)

static DWORD mmio_read(DWORD off) {
    return GadgetRd(0x7FC80000u + off);
}

static void mmio_write(DWORD off, DWORD val) {
    GadgetWr(val, 0x7FC80000u + off);
}

static const char* CMD_PATH   = "\\Device\\Harddisk0\\Partition1\\AuroraLaunch\\User\\Scripts\\Utility\\Toggle_240p\\cmd.txt";
static const char* STATE_PATH = "\\Device\\Harddisk0\\Partition1\\AuroraLaunch\\User\\Scripts\\Utility\\Toggle_240p\\state.txt";

typedef struct _KPATCH {
    DWORD addr;
    DWORD val;
} KPATCH;

static const KPATCH g_patches_240p[] = {
    { 0x800582C8, 0x2611920C },
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

__declspec(noinline) static void patch_word_icache(DWORD addr, DWORD val) {
    volatile DWORD* ptr = (volatile DWORD*)addr;
    *ptr = val;
    __dcbst(0, (void*)addr);
    __sync();
    __emit(0x7C001FAC); /* icbi r0, r3 (addr is in r3) */
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
    mmio_write(0x7910, 1);
    mmio_write(0x7900, 1);
    CGpuWrite(reg, val);
    mmio_write(0x7938, orig7938);
}

static int is_240p_active(void) {
    return (*(volatile DWORD*)0x800582C8 == 0x2611920Cu) ? 1 : 0;
}

static void enable_240p(void) {
    DWORD d, v_init;
    apply_patches(g_patches_240p, 7);
    hana_write_both(0x15, 0x2611920C);
    hana_write_both(0x16, 0x23232323);
    hana_write_both(0x26, 0x83814068);
    hana_write_both(0x28, 0x10B420FE);

    mmio_write(0x60E8, 1);
    mmio_write(0x65CC, 0x00010000);
    mmio_write(0x6010, 0x20B);
    mmio_write(0x6030, 1);

    d = mmio_read(0x6538) & 0xFFFFu;
    mmio_write(0x6538, (d << 16) | d);

    v_init = mmio_read(0x65C8);
    mmio_write(0x65DC, v_init);
    mmio_write(0x6578, 0);
    mmio_write(0x65CC, 0);
    mmio_write(0x60E8, 0);
}

static void disable_240p(void) {
    DWORD even_cnt, even_v_init, odd_v_init;
    apply_patches(g_patches_480i, 7);
    hana_write_both(0x15, 0x2651B20D);
    hana_write_both(0x16, 0x67672323);
    hana_write_both(0x26, 0x8381406A);
    hana_write_both(0x28, 0x10C420FF);

    mmio_write(0x60E8, 1);
    mmio_write(0x65CC, 0x00010000);
    mmio_write(0x6010, 0x20C);
    mmio_write(0x6030, 1);

    even_cnt = mmio_read(0x6538) & 0xFFFFu;
    mmio_write(0x6538, ((even_cnt + 1u) << 16) | even_cnt);

    even_v_init = mmio_read(0x65C8);
    odd_v_init = (even_v_init == 0x00040000u) ? 0x00060000u : (even_v_init + 0x00020000u);
    mmio_write(0x65DC, odd_v_init);
    mmio_write(0x6578, 0);
    mmio_write(0x65CC, 0);
    mmio_write(0x60E8, 0);
}

static void write_state_file(void) {
    ANSI_STRING as;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK iosb;
    HANDLE hFile = NULL;
    NTSTATUS st;
    LARGE_INTEGER_K zero;
    const char* txt = is_240p_active() ? "240p" : "480i";
    zero.QuadPart = 0;

    RtlInitAnsiString(&as, STATE_PATH);
    oa.RootDirectory = NULL;
    oa.ObjectName = &as;
    oa.Attributes = 0x40; /* OBJ_CASE_INSENSITIVE */

    /* Delete existing state.txt first so FATX always writes fresh 4 bytes */
    st = NtOpenFile(&hFile, 0x80110000, &oa, &iosb, 7, 0x1020);
    if (st >= 0 && hFile != NULL) {
        NtClose(hFile);
        hFile = NULL;
    }

    st = NtCreateFile(&hFile, 0x40100000 /* GENERIC_WRITE|SYNCHRONIZE */, &oa, &iosb,
                      NULL, 0x80 /* FILE_ATTRIBUTE_NORMAL */, 7, 2 /* FILE_CREATE */, 0x20 /* FILE_SYNCHRONOUS_IO_NONALERT */);
    if (st >= 0 && hFile != NULL) {
        NtWriteFile(hFile, NULL, NULL, NULL, &iosb, (PVOID)txt, 4, &zero);
        NtClose(hFile);
    }
}

static volatile DWORD g_diag[4] = { 0x54323430u, 0, 0, 0 }; /* "T240", pXamInputGetState, last_rc_btn, toggles */

static DWORD WINAPI WorkerThread(LPVOID lpParam) {
    LARGE_INTEGER_K delay50ms;
    LARGE_INTEGER_K delay2s;
    FN_XAM_INPUT_GET_STATE pXamInputGetState = NULL;
    int combo_240_held = 0;
    DWORD tick = 0;
    delay50ms.QuadPart = -500000LL;
    delay2s.QuadPart = -20000000LL;
    (void)lpParam;

    /* Give HDD a moment to mount at early DashLaunch boot */
    KeDelayExecutionThread(0, FALSE, &delay2s);
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

        /* Resolve XamInputGetState dynamically so t240.xex keeps zero static xam.xex imports */
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

        /* Check controller shortcut:
         * - LT + RT + L3 + R3 (L2 + R2 + L3 + R3) -> enable 240p
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
                if (rc == 0) {
                    if (((xs.Gamepad.wButtons & sticks) == sticks) &&
                        (xs.Gamepad.bLeftTrigger > 100) &&
                        (xs.Gamepad.bRightTrigger > 100)) {
                        now_240 = 1;
                        break;
                    }
                }
            }
            if (now_240 && !combo_240_held) {
                combo_240_held = 1;
                g_diag[3]++;
                enable_240p();
            } else if (!now_240) {
                combo_240_held = 0;
            }
        }

        if ((tick % 5u) != 0) {
            continue;
        }

        RtlInitAnsiString(&as, CMD_PATH);
        oa.RootDirectory = NULL;
        oa.ObjectName = &as;
        oa.Attributes = 0x40;

        /* First open read-only without deleting so we don't race Aurora's WriteFile */
        st = NtOpenFile(&hFile, 0x80100000 /* GENERIC_READ | SYNCHRONIZE */, &oa, &iosb, 7, 0x20);
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

        /* Now delete cmd.txt */
        hFile = NULL;
        st = NtOpenFile(&hFile, 0x80110000 /* GENERIC_READ | SYNCHRONIZE | DELETE */, &oa, &iosb, 7, 0x1020);
        if (st >= 0 && hFile != NULL) {
            NtClose(hFile);
        }

        if (buf[0] == 't') {
            if (is_240p_active()) disable_240p();
            else enable_240p();
            write_state_file();
        } else if (buf[0] == '1') {
            enable_240p();
            write_state_file();
        } else if (buf[0] == '0') {
            disable_240p();
            write_state_file();
        } else if (buf[0] == 's') {
            write_state_file();
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
            NtClose(hThread);
        }
    }
    return TRUE;
}
