#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <strsafe.h>

#define APP_NAME L"WindowsSettingsMenu"
#define APP_DISPLAY L"Windows Settings Menu"
#define REG_CS_PATH L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CommandStore\\shell\\WS."
#define REG_AUTOSTART L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define WM_TRAYICON (WM_APP + 1)
#define ID_TRAY_EXIT 4000
#define ID_TRAY_BASE 2000

typedef struct {
    const WCHAR *name;
    const WCHAR *uri;
} SETTING_ENTRY;

typedef struct {
    const WCHAR *name;
    int start;
    int end;
} CATEGORY;

static const SETTING_ENTRY g_items[] = {
    { L"Display",              L"ms-settings:display" },
    { L"Sound",                L"ms-settings:sound" },
    { L"Notifications && Actions", L"ms-settings:notifications" },
    { L"Power && Sleep",       L"ms-settings:powersleep" },
    { L"Storage",              L"ms-settings:storagesense" },
    { L"Multitasking",         L"ms-settings:multitasking" },
    { L"Clipboard",            L"ms-settings:clipboard" },
    { L"About",                L"ms-settings:about" },
    { L"Bluetooth",            L"ms-settings:bluetooth" },
    { L"Printers && Scanners", L"ms-settings:printers" },
    { L"Mouse",                L"ms-settings:mousetouchpad" },
    { L"Typing",               L"ms-settings:typing" },
    { L"AutoPlay",             L"ms-settings:autoplay" },
    { L"USB",                  L"ms-settings:usb" },
    { L"Network Status",       L"ms-settings:network" },
    { L"Wi-Fi",                L"ms-settings:network-wifi" },
    { L"Ethernet",             L"ms-settings:network-ethernet" },
    { L"VPN",                  L"ms-settings:network-vpn" },
    { L"Proxy",                L"ms-settings:network-proxy" },
    { L"Background",           L"ms-settings:personalization-background" },
    { L"Colors",               L"ms-settings:personalization-colors" },
    { L"Themes",               L"ms-settings:themes" },
    { L"Lock Screen",          L"ms-settings:lockscreen" },
    { L"Taskbar",              L"ms-settings:taskbar" },
    { L"Start Menu",           L"ms-settings:personalization-start" },
    { L"Apps && Features",     L"ms-settings:appsfeatures" },
    { L"Default Apps",         L"ms-settings:defaultapps" },
    { L"Startup Apps",         L"ms-settings:startupapps" },
    { L"Sign-in Options",      L"ms-settings:signinoptions" },
    { L"Family && Users",      L"ms-settings:otherusers" },
    { L"Date && Time",         L"ms-settings:dateandtime" },
    { L"Region",               L"ms-settings:regionformatting" },
    { L"Language",             L"ms-settings:language" },
    { L"Game Mode",            L"ms-settings:gaming-gamemode" },
    { L"Captures",             L"ms-settings:gaming-captures" },
    { L"Narrator",             L"ms-settings:easeofaccess-narrator" },
    { L"Magnifier",            L"ms-settings:easeofaccess-magnifier" },
    { L"High Contrast",        L"ms-settings:easeofaccess-highcontrast" },
    { L"Windows Security",     L"ms-settings:windowsdefender" },
    { L"Location Privacy",     L"ms-settings:privacy-location" },
    { L"Camera Privacy",       L"ms-settings:privacy-webcam" },
    { L"Windows Update",       L"ms-settings:windowsupdate" },
    { L"Backup",               L"ms-settings:backup" },
    { L"Troubleshoot",         L"ms-settings:troubleshoot" },
    { L"Recovery",             L"ms-settings:recovery" },
    { L"Activation",           L"ms-settings:activation" },
};

#define NUM_ITEMS (sizeof(g_items) / sizeof(g_items[0]))

static const CATEGORY g_categories[] = {
    { L"System",              0,  8 },
    { L"Devices",             8,  14 },
    { L"Network",             14, 19 },
    { L"Personalization",     19, 25 },
    { L"Apps",                25, 28 },
    { L"Accounts",            28, 30 },
    { L"Time && Language",    30, 33 },
    { L"Gaming",              33, 35 },
    { L"Accessibility",       35, 38 },
    { L"Privacy && Security", 38, 41 },
    { L"Update && Recovery",  41, 46 },
};

#define NUM_CATEGORIES (sizeof(g_categories) / sizeof(g_categories[0]))

static WCHAR g_exePath[MAX_PATH];
static BOOL g_isAdmin = FALSE;
static HWND g_hwnd = NULL;
static HINSTANCE g_hInst = NULL;
static HICON g_hIcon = NULL;

static BOOL IsAdmin(void)
{
    BOOL b = FALSE;
    PSID group = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuth, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &group)) {
        CheckTokenMembership(NULL, group, &b);
        FreeSid(group);
    }
    return b;
}

static void ElevateAndRun(const WCHAR *args)
{
    WCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    ShellExecute(NULL, L"runas", path, args, NULL, SW_SHOWNORMAL);
}

static HKEY OpenKey(HKEY root, const WCHAR *sub, REGSAM access, BOOL create)
{
    HKEY hk;
    if (create) {
        DWORD disp;
        RegCreateKeyEx(root, sub, 0, NULL, REG_OPTION_NON_VOLATILE, access, NULL, &hk, &disp);
    } else {
        RegOpenKeyEx(root, sub, 0, access, &hk);
    }
    return hk;
}

static void SetStr(HKEY hk, const WCHAR *name, const WCHAR *val)
{
    RegSetValueEx(hk, name, 0, REG_SZ, (const BYTE*)val, (lstrlen(val)+1)*sizeof(WCHAR));
}

static void InstallMenus(void)
{
    WCHAR exeArg[MAX_PATH + 64];
    StringCbPrintf(exeArg, sizeof(exeArg), L"\"%s\" --open", g_exePath);

    WCHAR subCmds[4096] = {0};
    WCHAR cmdName[64];
    for (int i = 0; i < NUM_ITEMS; i++) {
        StringCbPrintf(cmdName, sizeof(cmdName), L"WS.%03d", i);
        if (i > 0) StringCbCat(subCmds, sizeof(subCmds), L";");
        StringCbCat(subCmds, sizeof(subCmds), cmdName);
    }

    HKEY hk = OpenKey(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Classes\\Directory\\Background\\shell\\WindowsSettings",
        KEY_WRITE, TRUE);
    if (hk) {
        SetStr(hk, L"MUIVerb", APP_DISPLAY);
        SetStr(hk, L"Icon", L"%SystemRoot%\\System32\\imageres.dll,-2001");
        SetStr(hk, L"SubCommands", subCmds);
        SetStr(hk, L"Position", L"Top");
        RegCloseKey(hk);
    }

    for (int i = 0; i < NUM_ITEMS; i++) {
        WCHAR keyPath[512];
        StringCbPrintf(keyPath, sizeof(keyPath), L"%s%03d", REG_CS_PATH, i);
        hk = OpenKey(HKEY_LOCAL_MACHINE, keyPath, KEY_WRITE, TRUE);
        if (hk) {
            SetStr(hk, NULL, g_items[i].name);
            SetStr(hk, L"Icon", L"%SystemRoot%\\System32\\imageres.dll,-2001");
            HKEY hkCmd = OpenKey(hk, L"command", KEY_WRITE, TRUE);
            if (hkCmd) {
                WCHAR cmd[512];
                StringCbPrintf(cmd, sizeof(cmd), L"%s %s", exeArg, g_items[i].uri);
                SetStr(hkCmd, NULL, cmd);
                RegCloseKey(hkCmd);
            }
            RegCloseKey(hk);
        }
    }

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    HWND hwndProgman = FindWindow(L"Progman", NULL);
    if (hwndProgman) PostMessage(hwndProgman, WM_SETTINGCHANGE, 0, 0);
}

static void UninstallMenus(void)
{
    RegDeleteTree(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Classes\\Directory\\Background\\shell\\WindowsSettings");

    for (int i = 0; i < NUM_ITEMS; i++) {
        WCHAR keyPath[512];
        StringCbPrintf(keyPath, sizeof(keyPath), L"%s%03d", REG_CS_PATH, i);
        RegDeleteTree(HKEY_LOCAL_MACHINE, keyPath);
    }

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

static void InstallAutostart(void)
{
    WCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    WCHAR cmd[MAX_PATH + 32];
    StringCbPrintf(cmd, sizeof(cmd), L"\"%s\" --autostart", path);

    HKEY hk = OpenKey(HKEY_CURRENT_USER, REG_AUTOSTART, KEY_WRITE, TRUE);
    if (hk) {
        SetStr(hk, APP_NAME, cmd);
        RegCloseKey(hk);
    }
}

static void UninstallAutostart(void)
{
    HKEY hk = OpenKey(HKEY_CURRENT_USER, REG_AUTOSTART, KEY_WRITE, FALSE);
    if (hk) {
        RegDeleteValue(hk, APP_NAME);
        RegCloseKey(hk);
    }
}

static void OpenUri(const WCHAR *uri)
{
    ShellExecute(NULL, L"open", uri, NULL, NULL, SW_SHOWNORMAL);
}

static HMENU BuildTrayMenu(void)
{
    HMENU hMenu = CreatePopupMenu();
    for (int c = 0; c < NUM_CATEGORIES; c++) {
        HMENU hSub = CreatePopupMenu();
        for (int i = g_categories[c].start; i < g_categories[c].end; i++) {
            AppendMenu(hSub, MF_STRING, ID_TRAY_BASE + i, g_items[i].name);
        }
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSub, g_categories[c].name);
    }
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");
    return hMenu;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        NOTIFYICONDATA nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = g_hIcon;
        StringCbCopy(nid.szTip, sizeof(nid.szTip), APP_DISPLAY);
        Shell_NotifyIcon(NIM_ADD, &nid);
        g_hwnd = hwnd;
        break;
    }
    case WM_TRAYICON: {
        if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
            HMENU hMenu = BuildTrayMenu();
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);
            DestroyMenu(hMenu);
        }
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_TRAY_EXIT) {
            DestroyWindow(hwnd);
        } else if (id >= ID_TRAY_BASE && id < ID_TRAY_BASE + NUM_ITEMS) {
            OpenUri(g_items[id - ID_TRAY_BASE].uri);
        }
        break;
    }
    case WM_DESTROY: {
        NOTIFYICONDATA nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static void RunMessageLoop(void)
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static int RunTray(BOOL silent)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = g_hInst;
    wc.hIcon = g_hIcon;
    wc.lpszClassName = APP_NAME;
    if (!RegisterClass(&wc)) return 1;

    HWND hwnd = CreateWindow(APP_NAME, APP_DISPLAY, 0, 0, 0, 0, 0,
                             NULL, NULL, g_hInst, NULL);
    if (!hwnd) return 1;

    if (!silent) {
        NOTIFYICONDATA nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_INFO;
        nid.dwInfoFlags = NIIF_INFO;
        nid.uTimeout = 3000;
        StringCbCopy(nid.szInfo, sizeof(nid.szInfo),
            L"Right-click the tray icon to access Windows Settings");
        StringCbCopy(nid.szInfoTitle, sizeof(nid.szInfoTitle), APP_DISPLAY);
        Shell_NotifyIcon(NIM_MODIFY, &nid);
    }

    RunMessageLoop();
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR lpCmd, int nShow)
{
    (void)hPrev; (void)nShow;
    g_hInst = hInst;
    g_hIcon = LoadIcon(hInst, MAKEINTRESOURCE(101));

    GetModuleFileName(NULL, g_exePath, MAX_PATH);
    g_isAdmin = IsAdmin();

    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLine(), &argc);
    if (!argv) return 1;

    if (argc >= 3 && lstrcmpi(argv[1], L"--open") == 0) {
        OpenUri(argv[2]);
        LocalFree(argv);
        return 0;
    }

    if (argc >= 2 && lstrcmpi(argv[1], L"--install") == 0) {
        if (!g_isAdmin) { ElevateAndRun(L"--install"); LocalFree(argv); return 0; }
        InstallMenus();
        MessageBox(NULL,
            L"Desktop right-click menu installed!\n\n"
            L"The tray icon is also available.",
            APP_DISPLAY, MB_OK | MB_ICONINFORMATION);
        LocalFree(argv);
        return 0;
    }

    if (argc >= 2 && lstrcmpi(argv[1], L"--uninstall") == 0) {
        if (!g_isAdmin) { ElevateAndRun(L"--uninstall"); LocalFree(argv); return 0; }
        UninstallMenus();
        MessageBox(NULL,
            L"Desktop right-click menu removed.\n\n"
            L"The tray icon will still work.\n"
            L"Restart Explorer to complete removal.",
            APP_DISPLAY, MB_OK | MB_ICONINFORMATION);
        LocalFree(argv);
        return 0;
    }

    if (argc >= 2 && lstrcmpi(argv[1], L"--autostart-install") == 0) {
        InstallAutostart();
        if (g_isAdmin) InstallMenus();
        else ElevateAndRun(L"--install");
        MessageBox(NULL,
            L"Auto-start enabled!",
            APP_DISPLAY, MB_OK | MB_ICONINFORMATION);
        LocalFree(argv);
        return 0;
    }

    if (argc >= 2 && lstrcmpi(argv[1], L"--autostart-remove") == 0) {
        UninstallAutostart();
        MessageBox(NULL,
            L"Auto-start disabled.",
            APP_DISPLAY, MB_OK | MB_ICONINFORMATION);
        LocalFree(argv);
        return 0;
    }

    if (argc >= 2 && lstrcmpi(argv[1], L"--help") == 0) {
        MessageBox(NULL,
            L"Windows Settings Menu - System Tray Edition\n\n"
            L"Usage:\n"
            L"  (no args)     Show tray icon\n"
            L"  --tray        Show tray icon\n"
            L"  --install     Install desktop right-click menu (admin)\n"
            L"  --uninstall   Remove desktop right-click menu (admin)\n"
            L"  --autostart   Start tray icon silently (from auto-start)\n"
            L"  --autostart-install   Enable auto-start\n"
            L"  --autostart-remove    Disable auto-start\n"
            L"  --open <uri>  Open a settings page\n"
            L"  --help        Show this help",
            APP_DISPLAY, MB_OK | MB_ICONINFORMATION);
        LocalFree(argv);
        return 0;
    }

    /* --autostart or no args: show tray icon */
    BOOL silent = (argc >= 2 && lstrcmpi(argv[1], L"--autostart") == 0);
    LocalFree(argv);

    return RunTray(silent);
}
