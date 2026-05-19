# ESP32 Smart AC Controller

My room AC is a basic "dumb" unit with no WiFi or smart features, which meant I had to use the remote every single time I wanted to turn it on, off, or change the temperature. Even though my phone has an IR blaster, its range was terrible, so I still had to physically get up and point it at the AC.

Since I already had an unused ESP32, some IR components, and a few spare boards lying around, I decided to turn the AC into a smart device instead of replacing it entirely.

The ESP32 clones the exact IR signals from the original remote and connects to the internet through Blynk, letting me control the AC directly from my phone. Now I can change the temperature, turn it on or off remotely, or even start cooling my room before I get home so the room is already cold when I arrive.

What started as a small convenience project ended up becoming a pretty interesting exercise in reverse engineering IR protocols, signal cleanup, and embedded systems debugging.

![Demo](docs/demo.gif)

---

## How it works

The ESP32 connects to WiFi and listens for commands from the Blynk app. When I tap a button on my phone, it sends the exact same IR signal the original remote would send. The AC has no idea it's not the real remote.

```
Blynk App → Blynk Cloud → ESP32 → IR LED → AC unit
```

Three buttons currently in the app:
- **OFF** — turns the AC off
- **20°C** — turns it on at 20°C
- **24°C** — turns it on at 24°C

---

## IR signal reverse engineering

First thing I tried was finding a library or a signal database that already had my AC unit in it, spent a while on that and nothing worked. So I gave up and just decided to copy the raw signal directly from the remote and replay it exactly as-is, wired up an IR receiver to the ESP32, pointed the remote at it and started capturing.

The problem was noise, every capture looked slightly different and when I replayed it the AC just ignored it. The signal is 104 bits long and encodes the full AC state in one go, temperature, mode, fan speed, everything, so even a few bad bits and the AC rejects the whole thing. Manually comparing captures wasn't really an option so I just captured the same button press a bunch of times and had an AI read through all of them and pull out the common values. Whatever was consistent across all captures was the real signal and the rest was just noise, so from that I built a Frankenstein signal from those stable numbers and it worked first try. Did the same thing for OFF, 24°C, and 20°C.

For sending the signal I used a 2N2222 NPN transistor to drive the IR LED at a carrier frequency of 38kHz, GPIO pins on the ESP32 can only push around 12mA which isn't nearly enough to reach across a room, the transistor bumps that up to the ~100mA the LED actually needs. After that it was just the Blynk side, created the virtual pins, connected them to the ESP32, set up the buttons in the app and that was it.

---

## Hardware

- ESP32 WROOM-32
- VS1838B IR receiver (GPIO 18) — for capturing
- IR LED 940nm + 2N2222 transistor + resistors — for sending
- Powered over USB

Wiring diagram is in [`/docs`](docs/).

---

## Code

Two sketches:

**`ac_controller.ino`** — the main thing. Connects to Blynk, waits for button presses, fires the IR signal.

**`capture.ino`** — used this to capture the raw signals from the remote. Useful if you want to do the same for your own AC.

### Libraries

- `IRremote` by Armin Joachimsmeyer (tested on 4.5.0)
- `Blynk`
- `WiFi` (comes with ESP32 core)

### Setup

Fill in your credentials in `ac_controller.ino`:

```cpp
#define BLYNK_TEMPLATE_ID   "your_template_id"
#define BLYNK_TEMPLATE_NAME "your_template_name"
#define BLYNK_AUTH_TOKEN    "your_auth_token"

char ssid[] = "your_wifi";
char pass[] = "your_password";
```

In Blynk, create three button widgets set to **Push** mode:
- V0 → OFF
- V1 → 20°C
- V2 → 24°C

---

## Things I ran into

- GPIO pins on the ESP32 can't drive an IR LED directly, the signal is too weak to reach the AC unit — needed a transistor to amplify it
- Default IRremote buffer is too small for AC signals and silently cuts the capture short, had to set `RAW_BUFFER_LENGTH 400`

---

## What's next if I have time

- Design a PCB and print an enclosure so it doesn't live on a breadboard or perfboard forever
- Add more temperature presets
- Blynk scheduled automations
