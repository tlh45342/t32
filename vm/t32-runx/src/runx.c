/*
 * t32-runx.c
 *
 * Windows-only interactive application host for T32.
 *
 * 0.0.8 scope:
 *   - one VM / one vCPU
 *   - existing 80x25 T32 text display
 *   - polling keyboard through libt32vm
 *   - Machine: Start / Stop / Reset / Exit
 *   - Firmware: Select BIOS...
 *   - Machine: Load Program... at 0x00020000 for direct development
 *   - View: modeless Stats... CPU state window
 *   - Help: About... reports the running version
 *   - Disk: Attach / Detach Disk 0
 *   - red/green status lamp on the far right of the menu bar
 *   - guest POWER_OFF powers off the VM but leaves the application open
 */

#ifdef _WIN32

#include "t32.h"
#include "default_bios.h"

#include <windows.h>
#include <commdlg.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T32_RUNX_VERSION "0.0.8"
#define T32_RUNX_LOAD_ADDRESS UINT32_C(0x00001000)
#define T32_RUNX_PROGRAM_ADDRESS UINT32_C(0x00020000)
#define T32_RUNX_SLICE_INSTRUCTIONS UINT64_C(20000)
#define T32_RUNX_TIMER_ID 1u
#define T32_RUNX_TIMER_MS 10u
#define T32_RUNX_CELL_WIDTH 8
#define T32_RUNX_CELL_HEIGHT 16
#define T32_RUNX_MARGIN 8

#define IDM_MACHINE_LOAD_PROGRAM 1005
#define IDM_VIEW_STATS           1301
#define IDM_HELP_ABOUT           1401
#define IDM_MACHINE_START    1001
#define IDM_MACHINE_STOP     1002
#define IDM_MACHINE_RESET    1003
#define IDM_MACHINE_EXIT     1004
#define IDM_FIRMWARE_SELECT  1101
#define IDM_FIRMWARE_EMBEDDED 1102
#define IDM_DISK_ATTACH0     1201
#define IDM_DISK_DETACH0     1202
#define IDM_STATUS_LAMP      1901

typedef enum {
    T32_RUNX_BIOS_A_EMBEDDED = 0,
    T32_RUNX_BIOS_B_FILE = 1
} runx_bios_mode_t;

typedef struct {
    t32_machine_t *machine;
    HFONT font;
    HMENU menu;
    HBITMAP status_red;
    HBITMAP status_green;

    bool disk_attached;
    bool running;
    bool powered_off;
    bool direct_program;
    runx_bios_mode_t bios_mode;

    HWND stats_window;
    HWND stats_text;

    char program_path[MAX_PATH];
    char firmware_path[MAX_PATH];
    char disk_path[MAX_PATH];
    uint32_t load_address;
} runx_state_t;

static runx_state_t g_runx;

static LRESULT CALLBACK stats_window_proc(HWND window, UINT message,
                                          WPARAM wparam, LPARAM lparam);
static void refresh_stats_window(void);

static bool file_exists(const char *path)
{
    DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES &&
           (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

static void show_error(HWND owner, const char *message)
{
    MessageBoxA(owner, message, "t32-runx", MB_OK | MB_ICONERROR);
}

static void show_about(HWND owner)
{
    char message[256];

    snprintf(message, sizeof(message),
             "T32 RunX\n"
             "Version %s\n\n"
             "Windows interactive developer host for T32.\n"
             "One VM / one vCPU.\n\n"
             "BIOS A: embedded default\n"
             "BIOS B: external file override",
             T32_RUNX_VERSION);

    MessageBoxA(owner, message, "About T32 RunX",
                MB_OK | MB_ICONINFORMATION);
}

static bool choose_file(HWND owner, const char *title, const char *filter,
                        char *path, DWORD path_size)
{
    OPENFILENAMEA ofn;

    memset(&ofn, 0, sizeof(ofn));
    path[0] = '\0';

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrTitle = title;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = path;
    ofn.nMaxFile = path_size;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                OFN_HIDEREADONLY;

    return GetOpenFileNameA(&ofn) != 0;
}

static HBITMAP create_status_bitmap(COLORREF color)
{
    HDC screen;
    HDC memory;
    HBITMAP bitmap;
    HGDIOBJ old_bitmap;
    HBRUSH background;
    HBRUSH lamp;
    HPEN pen;
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;
    RECT rect = {0, 0, 16, 16};

    screen = GetDC(NULL);
    if (!screen)
        return NULL;

    memory = CreateCompatibleDC(screen);
    bitmap = CreateCompatibleBitmap(screen, 16, 16);
    ReleaseDC(NULL, screen);

    if (!memory || !bitmap) {
        if (memory)
            DeleteDC(memory);
        if (bitmap)
            DeleteObject(bitmap);
        return NULL;
    }

    old_bitmap = SelectObject(memory, bitmap);
    background = CreateSolidBrush(GetSysColor(COLOR_MENU));
    FillRect(memory, &rect, background);
    DeleteObject(background);

    lamp = CreateSolidBrush(color);
    pen = CreatePen(PS_SOLID, 1, RGB(64, 64, 64));
    old_brush = SelectObject(memory, lamp);
    old_pen = SelectObject(memory, pen);

    Ellipse(memory, 2, 2, 14, 14);

    SelectObject(memory, old_pen);
    SelectObject(memory, old_brush);
    SelectObject(memory, old_bitmap);

    DeleteObject(pen);
    DeleteObject(lamp);
    DeleteDC(memory);
    return bitmap;
}

static void update_status_lamp(HWND window)
{
    MENUITEMINFOA item;

    if (!g_runx.menu)
        return;

    memset(&item, 0, sizeof(item));
    item.cbSize = sizeof(item);
    item.fMask = MIIM_BITMAP;
    item.hbmpItem = g_runx.running ? g_runx.status_green
                                   : g_runx.status_red;

    SetMenuItemInfoA(g_runx.menu, IDM_STATUS_LAMP, FALSE, &item);
    DrawMenuBar(window);
}

static void update_ui(HWND window)
{
    SetWindowTextA(window, "t32-runx");
    update_status_lamp(window);
    refresh_stats_window();
}

static bool load_embedded_bios(HWND owner)
{
    if (!g_runx.machine)
        return false;

    if (!t32_write_memory(g_runx.machine,
                          T32_RUNX_LOAD_ADDRESS,
                          t32_runx_default_bios,
                          t32_runx_default_bios_size)) {
        show_error(owner, "Could not load the embedded T32 BIOS.");
        return false;
    }

    t32_set_pc(g_runx.machine, T32_RUNX_LOAD_ADDRESS);
    return true;
}

static bool create_machine(HWND owner)
{
    if (g_runx.machine) {
        t32_destroy(g_runx.machine);
        g_runx.machine = NULL;
    }

    g_runx.machine = t32_create(T32_DEFAULT_MEMORY_SIZE);
    if (!g_runx.machine) {
        show_error(owner, "Could not create the T32 machine.");
        return false;
    }

    if (g_runx.direct_program && g_runx.program_path[0]) {
        if (!t32_load_file(g_runx.machine,
                           g_runx.program_path,
                           T32_RUNX_PROGRAM_ADDRESS)) {
            char message[768];
            snprintf(message, sizeof(message),
                     "Could not load T32 program:\n\n%s",
                     g_runx.program_path);
            show_error(owner, message);
            return false;
        }
        t32_set_pc(g_runx.machine, T32_RUNX_PROGRAM_ADDRESS);
    } else if (g_runx.bios_mode == T32_RUNX_BIOS_B_FILE) {
        if (!g_runx.firmware_path[0]) {
            show_error(owner, "BIOS mode B requires an external BIOS file.");
            return false;
        }
        if (!t32_load_file(g_runx.machine,
                           g_runx.firmware_path,
                           g_runx.load_address)) {
            char message[768];
            snprintf(message, sizeof(message),
                     "Could not load T32 firmware:\n\n%s",
                     g_runx.firmware_path);
            show_error(owner, message);
            return false;
        }
        t32_set_pc(g_runx.machine, g_runx.load_address);
    } else {
        if (!load_embedded_bios(owner))
            return false;
    }

    g_runx.disk_attached = false;
    if (g_runx.disk_path[0]) {
        if (!t32_disk_attach(g_runx.machine, g_runx.disk_path)) {
            char message[768];
            snprintf(message, sizeof(message),
                     "Could not attach Disk 0:\n\n%s",
                     g_runx.disk_path);
            show_error(owner, message);
            return false;
        }
        g_runx.disk_attached = true;
    }

    g_runx.running = false;
    g_runx.powered_off = false;
    return true;
}

static void machine_start(HWND window)
{
    /*
     * STOP and guest POWER_OFF are hard/off states. Starting again creates a
     * fresh machine from the selected firmware and currently configured disk.
     */
    if (!g_runx.machine || g_runx.powered_off) {
        if (!create_machine(window))
            return;
    }

    g_runx.running = true;
    g_runx.powered_off = false;
    update_ui(window);
}

static void machine_stop(HWND window)
{
    /*
     * Host-enforced immediate stop: no guest notification, no graceful
     * shutdown request, no saving throw. The current framebuffer is left
     * visible for inspection. A later Start performs a fresh boot.
     */
    g_runx.running = false;
    g_runx.powered_off = true;
    if (g_runx.machine)
        t32_clear_platform_requests(g_runx.machine);
    update_ui(window);
}

static void machine_reset(HWND window)
{
    if (!create_machine(window))
        return;

    g_runx.running = true;
    InvalidateRect(window, NULL, TRUE);
    update_ui(window);
}

static void select_firmware(HWND window)
{
    char path[MAX_PATH];

    if (!choose_file(window,
                     "Select T32 BIOS/Firmware",
                     "T32 binary (*.bin)\0*.bin\0All files (*.*)\0*.*\0\0",
                     path, sizeof(path)))
        return;

    strncpy(g_runx.firmware_path, path,
            sizeof(g_runx.firmware_path) - 1);
    g_runx.firmware_path[sizeof(g_runx.firmware_path) - 1] = '\0';
    g_runx.bios_mode = T32_RUNX_BIOS_B_FILE;
    g_runx.direct_program = false;

    (void)create_machine(window);
    InvalidateRect(window, NULL, TRUE);
    update_ui(window);
}

static void use_embedded_firmware(HWND window)
{
    g_runx.bios_mode = T32_RUNX_BIOS_A_EMBEDDED;
    g_runx.firmware_path[0] = '\0';
    g_runx.direct_program = false;

    if (!create_machine(window))
        return;

    InvalidateRect(window, NULL, TRUE);
    update_ui(window);
}

static void load_program(HWND window)
{
    char path[MAX_PATH];

    if (!choose_file(window,
                     "Load T32 Program",
                     "T32 binary (*.bin)\0*.bin\0All files (*.*)\0*.*\0\0",
                     path, sizeof(path)))
        return;

    strncpy(g_runx.program_path, path, sizeof(g_runx.program_path) - 1);
    g_runx.program_path[sizeof(g_runx.program_path) - 1] = '\0';
    g_runx.direct_program = true;

    if (!create_machine(window))
        return;

    /* Loading is deliberately non-running; Start is a separate action. */
    InvalidateRect(window, NULL, TRUE);
    update_ui(window);
    refresh_stats_window();
}

static void format_stats_text(char *buffer, size_t size)
{
    unsigned r;
    size_t used = 0;
    t32_flags_t flags = {0};
    const char *state = "no machine";
    const char *reason = "";
    uint64_t instructions = 0;
    uint32_t pc = 0;

    if (g_runx.machine) {
        flags = t32_get_flags(g_runx.machine);
        state = t32_state_name(t32_get_state(g_runx.machine));
        reason = t32_get_halt_reason(g_runx.machine);
        instructions = t32_get_instruction_count(g_runx.machine);
        pc = t32_get_pc(g_runx.machine);
    }

    used += (size_t)snprintf(buffer + used, size - used,
                            "CPU Registers\r\n"
                            "------------------------------\r\n");
    for (r = 0; r < T32_REGISTER_COUNT && used < size; r += 2) {
        uint32_t left = g_runx.machine ? t32_get_register(g_runx.machine, r) : 0;
        uint32_t right = g_runx.machine ? t32_get_register(g_runx.machine, r + 1) : 0;
        used += (size_t)snprintf(buffer + used, size - used,
                                "r%-2u  %08X    r%-2u  %08X\r\n",
                                r, left, r + 1, right);
    }

    if (used < size)
        used += (size_t)snprintf(buffer + used, size - used,
                                "\r\nPC   %08X\r\n\r\n"
                                "Machine Status\r\n"
                                "------------------------------\r\n"
                                "State         %s\r\n"
                                "Instructions  %llu\r\n"
                                "Carry         %u\r\n"
                                "Zero          %u\r\n"
                                "Negative      %u\r\n"
                                "Overflow      %u\r\n"
                                "Reason        %s\r\n",
                                pc,
                                state,
                                (unsigned long long)instructions,
                                flags.carry ? 1u : 0u,
                                flags.zero ? 1u : 0u,
                                flags.negative ? 1u : 0u,
                                flags.overflow ? 1u : 0u,
                                reason && reason[0] ? reason : "-");
}

static void refresh_stats_window(void)
{
    char text[2048];

    if (!g_runx.stats_window || !g_runx.stats_text)
        return;

    format_stats_text(text, sizeof(text));
    SetWindowTextA(g_runx.stats_text, text);
}

static void show_stats_window(HWND owner)
{
    HINSTANCE instance = GetModuleHandleA(NULL);

    if (g_runx.stats_window) {
        ShowWindow(g_runx.stats_window, SW_SHOWNORMAL);
        SetForegroundWindow(g_runx.stats_window);
        refresh_stats_window();
        return;
    }

    g_runx.stats_window = CreateWindowExA(
        WS_EX_TOOLWINDOW,
        "T32RunXStatsWindow",
        "t32-runx CPU Stats",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 430, 390,
        owner, NULL, instance, NULL);

    if (!g_runx.stats_window) {
        show_error(owner, "Could not create the CPU Stats window.");
        return;
    }

    ShowWindow(g_runx.stats_window, SW_SHOW);
    UpdateWindow(g_runx.stats_window);
    refresh_stats_window();
}

static LRESULT CALLBACK stats_window_proc(HWND window, UINT message,
                                          WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WM_CREATE:
        g_runx.stats_text = CreateWindowExA(
            WS_EX_CLIENTEDGE,
            "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            0, 0, 0, 0,
            window, NULL, GetModuleHandleA(NULL), NULL);
        if (g_runx.stats_text) {
            SendMessageA(g_runx.stats_text, WM_SETFONT,
                         (WPARAM)g_runx.font, TRUE);
        }
        return 0;

    case WM_SIZE:
        if (g_runx.stats_text)
            MoveWindow(g_runx.stats_text, 0, 0,
                       LOWORD(lparam), HIWORD(lparam), TRUE);
        return 0;

    case WM_CLOSE:
        DestroyWindow(window);
        return 0;

    case WM_DESTROY:
        g_runx.stats_text = NULL;
        g_runx.stats_window = NULL;
        return 0;
    }

    return DefWindowProcA(window, message, wparam, lparam);
}

static void attach_disk0(HWND window)
{
    char path[MAX_PATH];

    if (!choose_file(window,
                     "Attach T32 Disk 0",
                     "Disk images (*.img)\0*.img\0All files (*.*)\0*.*\0\0",
                     path, sizeof(path)))
        return;

    strncpy(g_runx.disk_path, path, sizeof(g_runx.disk_path) - 1);
    g_runx.disk_path[sizeof(g_runx.disk_path) - 1] = '\0';

    if (!g_runx.machine && !create_machine(window))
        return;

    if (g_runx.machine) {
        t32_disk_detach(g_runx.machine);
        if (!t32_disk_attach(g_runx.machine, g_runx.disk_path)) {
            char message[768];
            snprintf(message, sizeof(message),
                     "Could not attach Disk 0:\n\n%s",
                     g_runx.disk_path);
            g_runx.disk_path[0] = '\0';
            g_runx.disk_attached = false;
            show_error(window, message);
            return;
        }
        g_runx.disk_attached = true;
    }
}

static void detach_disk0(void)
{
    if (g_runx.machine)
        t32_disk_detach(g_runx.machine);

    g_runx.disk_attached = false;
    g_runx.disk_path[0] = '\0';
}

static void guest_power_off(HWND window)
{
    if (g_runx.machine)
        t32_clear_platform_requests(g_runx.machine);

    g_runx.running = false;
    g_runx.powered_off = true;

    /*
     * POWER_OFF affects the virtual machine, not the host application.
     * The final framebuffer remains visible and the user may Start again.
     */
    update_ui(window);
}

static void run_vm_slice(HWND window)
{
    uint64_t count;

    if (!g_runx.machine || !g_runx.running)
        return;

    for (count = 0; count < T32_RUNX_SLICE_INSTRUCTIONS; ++count) {
        t32_step_result_t result = t32_step(g_runx.machine);
        if (result != T32_STEP_OK)
            break;
    }

    if (t32_video_is_dirty(g_runx.machine)) {
        t32_video_clear_dirty(g_runx.machine);
        InvalidateRect(window, NULL, FALSE);
    }

    refresh_stats_window();

    if (t32_power_off_requested(g_runx.machine)) {
        guest_power_off(window);
        return;
    }

    if (t32_reset_requested(g_runx.machine)) {
        t32_clear_platform_requests(g_runx.machine);
        machine_reset(window);
        return;
    }

    if (t32_get_state(g_runx.machine) == T32_STATE_HALTED ||
        t32_get_state(g_runx.machine) == T32_STATE_ERROR) {
        g_runx.running = false;
        g_runx.powered_off = true;
        update_ui(window);
    }
}

static void paint_display(HWND window)
{
    PAINTSTRUCT ps;
    HDC dc;
    RECT client;
    HGDIOBJ old_font;
    uint8_t video[T32_VIDEO_SIZE];
    unsigned row;

    dc = BeginPaint(window, &ps);
    GetClientRect(window, &client);

    FillRect(dc, &client, (HBRUSH)GetStockObject(BLACK_BRUSH));
    SetBkColor(dc, RGB(0, 0, 0));
    SetTextColor(dc, RGB(192, 192, 192));

    old_font = SelectObject(dc, g_runx.font);

    memset(video, ' ', sizeof(video));
    if (g_runx.machine)
        (void)t32_read_memory(g_runx.machine, T32_VIDEO_BASE,
                              video, sizeof(video));

    for (row = 0; row < T32_VIDEO_ROWS; ++row) {
        char line[T32_VIDEO_COLUMNS];
        unsigned column;

        for (column = 0; column < T32_VIDEO_COLUMNS; ++column) {
            uint8_t ch = video[row * T32_VIDEO_COLUMNS + column];
            line[column] = (ch >= 32 && ch <= 126) ? (char)ch : ' ';
        }

        TextOutA(dc,
                 T32_RUNX_MARGIN,
                 T32_RUNX_MARGIN + (int)row * T32_RUNX_CELL_HEIGHT,
                 line,
                 T32_VIDEO_COLUMNS);
    }

    SelectObject(dc, old_font);
    EndPaint(window, &ps);
}

static HMENU create_application_menu(void)
{
    HMENU root = CreateMenu();
    HMENU machine = CreatePopupMenu();
    HMENU firmware = CreatePopupMenu();
    HMENU disk = CreatePopupMenu();
    HMENU view = CreatePopupMenu();
    HMENU help = CreatePopupMenu();
    MENUITEMINFOA status;

    AppendMenuA(machine, MF_STRING, IDM_MACHINE_START, "&Start");
    AppendMenuA(machine, MF_STRING, IDM_MACHINE_STOP, "S&top");
    AppendMenuA(machine, MF_STRING, IDM_MACHINE_RESET, "&Reset");
    AppendMenuA(machine, MF_SEPARATOR, 0, NULL);
    AppendMenuA(machine, MF_STRING, IDM_MACHINE_LOAD_PROGRAM,
                "&Load Program...");
    AppendMenuA(machine, MF_SEPARATOR, 0, NULL);
    AppendMenuA(machine, MF_STRING, IDM_MACHINE_EXIT, "E&xit");

    AppendMenuA(firmware, MF_STRING, IDM_FIRMWARE_EMBEDDED,
                "Use &Embedded BIOS (A)");
    AppendMenuA(firmware, MF_STRING, IDM_FIRMWARE_SELECT,
                "Select External BIOS... (&B)");

    AppendMenuA(disk, MF_STRING, IDM_DISK_ATTACH0,
                "&Attach Disk 0...");
    AppendMenuA(disk, MF_STRING, IDM_DISK_DETACH0,
                "&Detach Disk 0");

    AppendMenuA(view, MF_STRING, IDM_VIEW_STATS, "&Stats...");
    AppendMenuA(help, MF_STRING, IDM_HELP_ABOUT, "&About...");

    AppendMenuA(root, MF_POPUP, (UINT_PTR)machine, "&Machine");
    AppendMenuA(root, MF_POPUP, (UINT_PTR)firmware, "&Firmware");
    AppendMenuA(root, MF_POPUP, (UINT_PTR)disk, "&Disk");
    AppendMenuA(root, MF_POPUP, (UINT_PTR)view, "&View");
    AppendMenuA(root, MF_POPUP, (UINT_PTR)help, "&Help");

    memset(&status, 0, sizeof(status));
    status.cbSize = sizeof(status);
    status.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STATE | MIIM_BITMAP;
    status.wID = IDM_STATUS_LAMP;
    status.fType = MFT_RIGHTJUSTIFY;
    status.fState = MFS_ENABLED;
    status.hbmpItem = g_runx.status_red;
    InsertMenuItemA(root, GetMenuItemCount(root), TRUE, &status);

    return root;
}

static LRESULT CALLBACK runx_window_proc(HWND window, UINT message,
                                         WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case IDM_MACHINE_LOAD_PROGRAM:
            load_program(window);
            return 0;
        case IDM_VIEW_STATS:
            show_stats_window(window);
            return 0;
        case IDM_HELP_ABOUT:
            show_about(window);
            return 0;
        case IDM_MACHINE_START:
            machine_start(window);
            return 0;
        case IDM_MACHINE_STOP:
            machine_stop(window);
            return 0;
        case IDM_MACHINE_RESET:
            machine_reset(window);
            return 0;
        case IDM_MACHINE_EXIT:
            DestroyWindow(window);
            return 0;
        case IDM_FIRMWARE_EMBEDDED:
            use_embedded_firmware(window);
            return 0;
        case IDM_FIRMWARE_SELECT:
            select_firmware(window);
            return 0;
        case IDM_DISK_ATTACH0:
            attach_disk0(window);
            return 0;
        case IDM_DISK_DETACH0:
            detach_disk0();
            return 0;
        case IDM_STATUS_LAMP:
            /* Display-only status indicator. */
            return 0;
        }
        break;

    case WM_CHAR:
        if (g_runx.machine && g_runx.running) {
            unsigned character = (unsigned)wparam;
            if (character == '\r')
                character = '\n';
            if (character <= 0x7f)
                (void)t32_keyboard_push(g_runx.machine, (uint8_t)character);
        }
        return 0;

    case WM_TIMER:
        if (wparam == T32_RUNX_TIMER_ID)
            run_vm_slice(window);
        return 0;

    case WM_PAINT:
        paint_display(window);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_CLOSE:
        DestroyWindow(window);
        return 0;

    case WM_DESTROY:
        KillTimer(window, T32_RUNX_TIMER_ID);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcA(window, message, wparam, lparam);
}

int main(int argc, char **argv)
{
    HINSTANCE instance = GetModuleHandleA(NULL);
    WNDCLASSA wc;
    HWND window;
    MSG message;
    RECT desired;
    int client_width;
    int client_height;

    memset(&g_runx, 0, sizeof(g_runx));
    g_runx.load_address = T32_RUNX_LOAD_ADDRESS;
    g_runx.bios_mode = T32_RUNX_BIOS_A_EMBEDDED;

    if (argc == 2 &&
        (strcmp(argv[1], "--version") == 0 ||
         strcmp(argv[1], "-v") == 0)) {
        MessageBoxA(NULL, "t32-runx 0.0.8", "t32-runx",
                    MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    if (argc >= 2) {
        strncpy(g_runx.firmware_path, argv[1],
                sizeof(g_runx.firmware_path) - 1);
        g_runx.firmware_path[sizeof(g_runx.firmware_path) - 1] = '\0';
        g_runx.bios_mode = T32_RUNX_BIOS_B_FILE;
    }

    if (argc >= 3)
        g_runx.load_address = (uint32_t)strtoul(argv[2], NULL, 0);

    if (file_exists("disk.img")) {
        strncpy(g_runx.disk_path, "disk.img",
                sizeof(g_runx.disk_path) - 1);
        g_runx.disk_path[sizeof(g_runx.disk_path) - 1] = '\0';
    }

    g_runx.status_red = create_status_bitmap(RGB(220, 32, 32));
    g_runx.status_green = create_status_bitmap(RGB(32, 180, 64));

    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = runx_window_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "T32RunXWindow";

    if (!RegisterClassA(&wc)) {
        show_error(NULL, "Could not register the t32-runx window class.");
        return 1;
    }

    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = stats_window_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "T32RunXStatsWindow";
    if (!RegisterClassA(&wc)) {
        show_error(NULL, "Could not register the t32-runx stats window class.");
        return 1;
    }

    client_width = (int)T32_VIDEO_COLUMNS * T32_RUNX_CELL_WIDTH +
                   T32_RUNX_MARGIN * 2;
    client_height = (int)T32_VIDEO_ROWS * T32_RUNX_CELL_HEIGHT +
                    T32_RUNX_MARGIN * 2;
    desired.left = 0;
    desired.top = 0;
    desired.right = client_width;
    desired.bottom = client_height;
    AdjustWindowRect(&desired, WS_OVERLAPPEDWINDOW, TRUE);

    g_runx.menu = create_application_menu();

    window = CreateWindowExA(
        0,
        "T32RunXWindow",
        "t32-runx",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        desired.right - desired.left,
        desired.bottom - desired.top,
        NULL, g_runx.menu, instance, NULL);

    if (!window) {
        show_error(NULL, "Could not create the t32-runx window.");
        return 1;
    }

    g_runx.font = CreateFontA(
        -T32_RUNX_CELL_HEIGHT, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN,
        "Consolas");

    if (!g_runx.font)
        g_runx.font = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);

    (void)create_machine(window);

    update_ui(window);
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    SetTimer(window, T32_RUNX_TIMER_ID, T32_RUNX_TIMER_MS, NULL);

    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    if (g_runx.font &&
        g_runx.font != (HFONT)GetStockObject(SYSTEM_FIXED_FONT))
        DeleteObject(g_runx.font);
    if (g_runx.status_red)
        DeleteObject(g_runx.status_red);
    if (g_runx.status_green)
        DeleteObject(g_runx.status_green);
    if (g_runx.machine)
        t32_destroy(g_runx.machine);

    return (int)message.wParam;
}

#else

int main(void)
{
    return 1;
}

#endif
