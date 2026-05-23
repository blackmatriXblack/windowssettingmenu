Windows Settings Context Menu
================================
Adds a "Windows Settings" cascading menu to the right-click menu of
Desktop background and "This PC" system icon.

Quick access to 46 Windows Settings pages in 12 categories.

INSTALLATION:
  1. Right-click "Install_Windows_Settings_Menu.reg"
  2. Select "Merge" (or double-click it)
  3. Click "Yes" on the UAC (User Account Control) prompt
  4. Click "OK" to confirm
  5. Right-click on Desktop or "This PC" to see the new menu

UNINSTALLATION:
  1. Right-click "Uninstall_Windows_Settings_Menu.reg"
  2. Select "Merge"
  3. Click "Yes" on UAC prompt
  4. Click "OK" to confirm

REFRESH EXPLORER (if menus don't appear immediately):
  - Press Ctrl+Shift+Esc to open Task Manager
  - Find "Windows Explorer" in the list
  - Right-click it and select "Restart"
  - Or simply log off and log back in

MENU ITEMS:
  System:
    Display, Sound, Notifications & Actions, Power & Sleep,
    Storage, Multitasking, Clipboard, About

  Devices:
    Bluetooth, Printers & Scanners, Mouse, Typing,
    AutoPlay, USB

  Network:
    Network Status, Wi-Fi, Ethernet, VPN, Proxy

  Personalization:
    Background, Colors, Themes, Lock Screen,
    Taskbar, Start Menu

  Apps:
    Apps & Features, Default Apps, Startup Apps

  Accounts:
    Sign-in Options, Family & Other Users

  Time & Language:
    Date & Time, Region, Language

  Gaming:
    Game Mode, Captures

  Accessibility:
    Narrator, Magnifier, High Contrast

  Privacy & Security:
    Windows Security, Location Privacy, Camera Privacy

  Update & Recovery:
    Windows Update, Backup, Troubleshoot, Recovery, Activation

REQUIREMENTS:
  - Windows 8, 8.1, 10, or 11
  - Administrator privileges (for installation)
  - All 46 settings pages use standard ms-settings: URIs

CUSTOMIZATION:
  To add or remove items, edit the .reg file:
  1. Add/remove CommandStore entries in the file
  2. Update the "SubCommands" value (semicolon-separated list)
  3. Re-run the .reg file

NOTE:
  The companion program "WindowsSettingsMenu.exe" is an alternative
  tool that can also install these menus programmatically.
  Use --install flag (run as Administrator).

TROUBLESHOOTING:
  - Menus not showing? Restart Explorer (see above)
  - Settings page not opening? Some ms-settings: URIs
    may not be available on your Windows version
  - Permission denied? Make sure to run the .reg file
    as Administrator
