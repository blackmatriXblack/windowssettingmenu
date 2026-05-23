This is a comprehensive technical documentation for the **WindowsSettingsMenu** utility. It is written in a professional, dual-language format (English/Chinese) and covers architectural design, implementation details, and deployment strategies.

---

# WindowsSettingsMenu: Technical Documentation & System Overview
# WindowsSettingsMenu：技术文档与系统概览

## 1. Executive Summary / 项目综述
### English
**WindowsSettingsMenu** is a high-performance, low-latency utility written in native C that provides a centralized bridge to the Windows 10/11 "ms-settings" URI scheme. It operates via two primary modalities: a persistent system tray application (System Tray) and a deep-integrated shell extension (Desktop Context Menu). By leveraging the Win32 API and the Windows Registry's `CommandStore` pattern, it offers an efficient alternative to navigating the standard Windows Settings UI.

### 中文
**WindowsSettingsMenu** 是一款采用原生 C 语言编写的高性能、低延迟实用工具，为 Windows 10/11 的 “ms-settings” URI 方案提供了一个集中的调用桥梁。该程序通过两种主要模式运行：持久化的系统托盘应用程序和深度集成的外壳扩展（桌面右键菜单）。通过利用 Win32 API 和 Windows 注册表的 `CommandStore` 模式，它为用户提供了一种比标准 Windows 设置 UI 更快捷的导航方式。

---

## 2. Technical Architecture / 技术架构

### 2.1 Component Interoperability / 组件交互
- **Core Engine (C/Win32):** Minimalist overhead, bypassing heavy frameworks like .NET or Electron.
- **Registry Management:** Orchestrates `HKEY_LOCAL_MACHINE` for shell integration and `HKEY_CURRENT_USER` for persistence.
- **Protocol Deep-linking:** Uses `ShellExecute` to dispatch `ms-settings:` protocols directly to the Universal Windows Platform (UWP) settings host.

### 中文核心架构
- **核心引擎 (C/Win32):** 极简开销，绕过 .NET 或 Electron 等沉重框架。
- **注册表管理:** 编排 `HKEY_LOCAL_MACHINE` 以实现外壳集成，并管理 `HKEY_CURRENT_USER` 以实现开机自启。
- **协议深度链接:** 使用 `ShellExecute` 将 `ms-settings:` 协议直接分发到通用 Windows 平台 (UWP) 设置宿主程序。

---

## 3. Implementation Details / 实现细节

### 3.1 Registry-Based Shell Extension / 基于注册表的外壳扩展
### English
The application implements a hierarchical context menu using the `CommandStore` infrastructure. This allows for a nested menu structure without the need for a COM-based shell extension DLL, improving system stability.
- **Path:** `SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\WS.*`
- **Methodology:** Every setting is mapped to a unique sub-command, which triggers the application binary with an `--open` flag.

### 中文实现细节
该程序利用 `CommandStore` 架构实现了一个分层右键菜单。这允许创建嵌套菜单结构，而无需编写基于 COM 的外壳扩展 DLL，从而提高了系统稳定性。
- **路径:** `SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\WS.*`
- **方法论:** 每个设置项都映射到一个唯一的子命令，该子命令通过 `--open` 标识触发程序二进制文件。

### 3.2 Privilege Elevation Logic / 权限提升逻辑
### English
Since writing to `HKEY_LOCAL_MACHINE` requires administrative privileges, the code includes a self-elevation mechanism using the `runas` verb within `ShellExecute`.
```c
if (!g_isAdmin) { ElevateAndRun(L"--install"); return 0; }
```
### 中文权限逻辑
由于写入 `HKEY_LOCAL_MACHINE` 需要管理员权限，代码包含了使用 `ShellExecute` 中的 `runas` 谓词实现的自动提权机制。

---

## 4. Data Structures / 数据结构

### English
The application utilizes static structured arrays to maintain the relationship between display names and URI endpoints.
### 中文
应用程序利用静态结构化数组来维护显示名称与 URI 端点之间的映射关系。

```c
typedef struct {
    const WCHAR *name; // Display name / 显示名称
    const WCHAR *uri;  // Protocol URI / 协议路径
} SETTING_ENTRY;

typedef struct {
    const WCHAR *name; // Category label / 类别标签
    int start;         // Start index / 起始索引
    int end;           // End index / 结束索引
} CATEGORY;
```

---

## 5. Command Line Interface (CLI) / 命令行界面说明

| Argument / 参数 | Description (English) | 描述 (中文) |
| :--- | :--- | :--- |
| `--install` | Registers the desktop context menu (Requires Admin). | 注册桌面右键菜单（需要管理员权限）。 |
| `--uninstall` | Removes all registry entries and cleans shell cache. | 移除所有注册表项并清理外壳缓存。 |
| `--autostart` | Launches the tray icon silently. | 静默启动托盘图标。 |
| `--autostart-install` | Configures the app to launch on Windows login. | 配置程序在 Windows 登录时自动启动。 |
| `--open <uri>` | Directly opens a specific settings URI. | 直接打开特定的设置 URI。 |

---

## 6. Message Loop & Window Procedure / 消息循环与窗口过程

### English
The tray icon functionality is handled by a hidden background window (`HWND`). It listens for the `WM_TRAYICON` message to spawn the `TrackPopupMenu`.
- **Interactivity:** Supports both Left and Right clicks for maximum accessibility.
- **Resource Management:** Automatically cleans up `NOTIFYICONDATA` upon `WM_DESTROY`.

### 中文
托盘图标功能由一个隐藏的后台窗口 (`HWND`) 处理。它监听 `WM_TRAYICON` 消息以弹出 `TrackPopupMenu`。
- **交互性:** 同时支持左键和右键点击，以实现最大的易用性。
- **资源管理:** 在收到 `WM_DESTROY` 消息时自动清理 `NOTIFYICONDATA` 结构。

---

## 7. Build Instructions / 构建指南

### English
To compile the application using the **MinGW-w64** toolchain, execute the following command:
### 中文
若要使用 **MinGW-w64** 工具链编译此程序，请执行以下命令：

```bash
gcc WindowsSettingsMenu.c -o WindowsSettingsMenu.exe \
    -mwindows -lunicode -lshell32 -lshlwapi -luser32 -ladvapi32
```

### English (MSVC)
If using the **Microsoft Visual C++** compiler:
### 中文 (MSVC)
如果使用 **Microsoft Visual C++** 编译器：

```bash
cl WindowsSettingsMenu.c /O2 /D "UNICODE" /D "_UNICODE" \
   shell32.lib shlwapi.lib user32.lib advapi32.lib /link /subsystem:windows
```

---

## 8. Security & Best Practices / 安全与最佳实践

1. **UAC Awareness:** The program explicitly checks for tokens before attempting registry modification.
2. **Buffer Safety:** Uses `StringCbPrintf` and `StringCbCopy` to prevent buffer overflow vulnerabilities.
3. **Low Footprint:** The application typically consumes < 2MB of RAM while idling in the system tray.

### 中文安全说明
1. **UAC 感知:** 程序在尝试修改注册表之前会明确检查权限令牌。
2. **缓冲区安全:** 使用 `StringCbPrintf` 和 `StringCbCopy` 来防止缓冲区溢出漏洞。
3. **低资源占用:** 应用程序在系统托盘静默运行时，通常占用内存小于 2MB。

---

## 9. Conclusion / 结语
### English
**WindowsSettingsMenu** is an optimized solution for power users and IT professionals who require rapid access to system configuration. By bypassing the sluggish "Start Menu" search and the multiple clicks required by the Settings Home page, it significantly enhances workflow efficiency.

### 中文
**WindowsSettingsMenu** 是为需要快速访问系统配置的高级用户和 IT 专业人员提供的优化解决方案。通过绕过反应迟缓的“开始菜单”搜索和“设置”主页所需的多次点击，它显著提高了工作流效率。
