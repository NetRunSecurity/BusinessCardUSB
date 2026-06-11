# Firmware

Firmware for the USB Business Card (CH552G), built with the
[ch55xduino](https://github.com/DeqingSun/ch55xduino) core under **Arduino IDE 2.x**.

Full hardware, pinout and schematics:
<https://wiki.netrunsecurity.com/electronics/sao/business-card/>

## Folders

| Folder              | What it does                                      |
| ------------------- | ------------------------------------------------- |
| `rubber-ducky/`     | USB HID keystroke injection (opens a URL on plug-in) |
| `touchpad-testing/`   | Capacitive Touch-Key + LED demo                   |
| `ctf/`       | USB serial CLI with hidden flags                  |
| `usb-uart-bridge/`  | USB ↔ UART bridge for debugging IoT targets       |

## Toolchain setup (once)

1. Install **Arduino IDE 2.x** : <https://www.arduino.cc/en/software>
2. **File → Preferences → Additional Boards Manager URLs**, add:  `https://raw.githubusercontent.com/DeqingSun/ch55xduino/ch55xduino/package_ch55xduino_mcs51_index.json`
3. **Tools → Board → Boards Manager** → search `ch55xduino` → install.

## Board settings (Tools menu)

```
Board         : CH552
Clock Source  : 16 MHz (internal)
Upload method : USB
USB Settings  : USER CODE w/ 148B USB ram
```

> ⚠️ **`USER CODE w/ 148B USB ram` is mandatory.** With any other USB setting,
> the HID / serial headers are not found and compilation fails. This is the #1 gotcha.

## Compiling

**HID / serial sketches** (`rubber-ducky`, `serial-ctf`) need the USB source files
that ship with the ch55xduino examples:

1. **File → Examples → CH55xDuino → HidKeyboard** (or the matching USB example).
2. Replace the sketch code with the `.ino` from this repo.
3. **Sketch → Verify/Compile**.

Opening the example first is required, a blank sketch is missing the USB header
sources and will not compile.

**Plain sketches** (`touch-led-test`): no example needed. Open the `.ino` and
Verify/Compile directly.

## Flashing

1. Hold **SW2 (BOOT)**, plug the card in, release after ~1 s.
2. Device enumerates as `VID_4348 PID_55E0`.
3. **Sketch → Upload**, or flash the built binary with
   [WCHISPTool](https://www.wch-ic.com/downloads/WCHISPTool_Setup_exe.html).
4. Unplug / replug to run.