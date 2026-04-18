# 🔓 pin-brute

> ESP32-S2 Mini — USB HID PIN Brute Force Tool with Web Portal  
> **Created by Nihal MP**

---

## 📦 Files

```
pin-brute/
├── pin-brute.ino   ← Main Arduino sketch
└── README.md       ← This file
```

---

## 🛠 Hardware Required

| Component | Details |
|-----------|---------|
| Board | ESP32-S2 Mini (LOLIN S2 Mini) |
| Connection | USB-C (native USB-CDC) |
| Target | Any device with USB keyboard input |

---

## ⚙️ Setup & Flash

1. Open `pin-brute.ino` in **Arduino IDE**
2. Install board support:
   - Go to `File → Preferences → Additional Board URLs`
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Go to `Tools → Board Manager` → search **esp32** → install
3. Select board: `Tools → Board → ESP32S2 Dev Module` or **LOLIN S2 Mini**
4. Select port: `Tools → Port → (your ESP32 port)`
5. Upload the sketch

---

## 🌐 Web Portal Access

After flashing, the ESP32 creates a WiFi hotspot:

| Setting | Value |
|---------|-------|
| WiFi Name (SSID) | `ESP32-BruteForce` |
| Password | `12345678` |
| Portal URL | `http://192.168.4.1` |

Connect your phone or laptop to the WiFi, then open the URL in a browser.

---

## 🖥️ Web Portal Features

### 💥 Brute Force Config
Configure and launch PIN brute force attacks via USB HID keyboard injection.

| Field | Description |
|-------|-------------|
| PIN Length | 4-digit (0000–9999) or 6-digit (000000–999999) |
| Start From | Resume from a specific PIN number |
| Delay (ms) | Time between each PIN attempt (500–10000 ms) |

**Buttons:**
- ▶ **START ATTACK** — Begin brute forcing
- ■ **STOP** — Stop the attack
- ⏸ **PAUSE / RESUME** — Pause or resume

**Status display** shows: current PIN, progress %, PIN length, delay

---

### ⌨️ Custom Command Input
Type any text or shell command — the ESP32 will inject it as HID keystrokes on the target machine and press Enter.

**Example inputs:**

| Input | Effect on target |
|-------|-----------------|
| `notepad` | Opens Notepad (Windows) |
| `calc` | Opens Calculator (Windows) |
| `ls -la` | Lists files (Linux terminal) |
| `whoami` | Shows current user |
| `ifconfig` | Shows network info (Linux) |
| `echo Hello` | Prints text |

> ⚠️ The target must have a focused input field or open terminal for this to work.

---

### 🖥️ Open Terminal
One-click buttons to open a terminal on the target machine via HID:

| Button | Shortcut Used | Target OS |
|--------|--------------|-----------|
| 🐧 **Linux Terminal** | `Ctrl + Alt + T` | Ubuntu, Kali, Mint, Debian |
| 💻 **Windows CMD** | `Win + R` → `cmd.exe` | Windows 7/10/11 |
| ⚡ **PowerShell** | `Win + R` → `powershell` | Windows 10/11 |

**Recommended workflow:**
1. Press the terminal button for target OS
2. Wait 1–2 seconds for terminal to open
3. Use **Custom Command Input** to run commands in the open terminal

---

## 📊 Serial Monitor Output

Connect via Serial Monitor at **115200 baud** to see live logs:

```
=================================
 ESP32 Brute Force — by Nihal MP
=================================
Connect WiFi : ESP32-BruteForce
Open         : http://192.168.4.1
[Nihal MP] Tried PIN: 0001
[Nihal MP] Tried PIN: 0002
[Nihal MP] Opening Linux terminal...
[Nihal MP] Opening Windows CMD...
[Nihal MP] Opening PowerShell...
[Nihal MP] Sending custom command: whoami
[Nihal MP] Custom command sent.
[Nihal MP] Brute force complete — all PINs tried.
```

---

## ⚠️ Disclaimer

This tool is for **educational and authorized security testing purposes only**.  
Only use on devices you own or have explicit written permission to test.  
Unauthorized use is illegal and unethical.

---

## 👤 Author

**Created by Nihal MP**  
ESP32-S2 Mini · USB HID Keyboard · Web Portal
