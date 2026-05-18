# ESP32 Smart AC Controller

My room AC is a basic “dumb” unit with no WiFi or smart features, which meant I had to use the remote every single time I wanted to turn it on, off, or change the temperature. Even though my phone has an IR blaster, its range was terrible, so I still had to physically get up and point it at the AC.

Since I already had an unused ESP32, some IR components, and a few spare boards lying around, I decided to turn the AC into a smart device instead of replacing it entirely.

The ESP32 clones the exact IR signals from the original remote and connects to the internet through Blynk, letting me control the AC directly from my phone. Now I can change the temperature, turn it on or off remotely, or even start cooling my room before I get home from work so the room is already cold when I arrive.

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

## The annoying part — IR signal reverse engineering

This took way longer than expected. AC remotes don't use simple on/off codes like a TV remote. They send a full 104-bit signal every time that encodes the temperature, mode, fan speed, everything. No library had support for my specific AC unit so I had to figure it out myself.

The first problem was noise. Every time I captured the signal it looked slightly different — timings were off by 50-100µs each capture. When I fed those directly to the ESP32 to replay, the AC just ignored it.

What actually worked: I captured the same button press 6 times, compared all of them, and looked for what stayed consistent. The bits that were the same across every capture were the real signal. The ones that varied were just noise. Then I averaged the noisy timing values to get clean numbers.

Ended up with:
- Short pulse (0 bit) → **550µs**
- Long pulse (1 bit) → **1700µs**
- Header → **9000µs / 4500µs**

That clean signal worked first try.

Raw captures are in [`/captures`](captures/) if you want to see what the noisy vs clean data looks like.

---

## Hardware

- ESP32 WROOM-32
- VS1838B IR receiver (GPIO 18) — for capturing
- IR LED 940nm + 2N2222 transistor + resistors — for sending
- Powered over USB

The transistor matters — I tried driving the IR LED directly from the GPIO pin first and the AC never responded. GPIO pins on the ESP32 can only push about 12mA, the LED needs closer to 100mA to actually reach across the room.

Wiring diagram is in [`/docs`](docs/).

---

## Code

Two sketches:

**`src/ac_controller/`** — the main thing. Connects to Blynk, waits for button presses, fires the IR signal.

**`src/capture/`** — used this to capture the raw signals from the remote. Useful if you want to do the same for your own AC.

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

- Single captures are too noisy to use directly — capture the same signal multiple times and average the stable timings

---

## What's next if i have time

- Design a PCB and print an enclosure so it doesn't live on a breadboard or perfboard forever
- Add more temperature presets
- Blynk scheduled automations

---


