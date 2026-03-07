# ESP32 Smart Waste Sorter ♻️🤖

An IoT-enabled smart waste sorting system that automatically classifies and routes waste into wet, dry, and metal bins. Built on the ESP32 microcontroller with WiFi connectivity for cloud monitoring via ThingSpeak.

---
![Platform](https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge&logo=espressif)
![Framework](https://img.shields.io/badge/Framework-Arduino-00979D?style=for-the-badge&logo=arduino)
![IoT](https://img.shields.io/badge/IoT-ThingSpeak-orange?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
![Language](https://img.shields.io/badge/Language-C%2B%2B-red?style=for-the-badge&logo=cplusplus)
![Bins](https://img.shields.io/badge/Bins-Wet%20%7C%20Dry%20%7C%20Metal-purple?style=for-the-badge)

## Project Description 📝

This project implements an autonomous waste sorting mechanism that detects incoming waste, classifies it using metal detection and moisture sensing, and directs it to the appropriate bin using a stepper motor and servo-controlled gate. Bin fill levels are monitored in real time with ultrasonic sensors, and data is uploaded to ThingSpeak for remote monitoring and alerting.

---

## Features 🌟

| Feature | Description |
|---------|-------------|
| **WiFi + ThingSpeak IoT** | Uploads bin levels and alarm status to ThingSpeak every 15 seconds |
| **Three-bin sorting** | Wet, dry, and metal waste bins with automatic classification |
| **Stepper motor sorter** | Rotates to align with the target bin (120° wet, 240° metal) |
| **Servo gate** | Opens/closes to drop waste into the selected bin |
| **Ultrasonic bin sensors** | Monitors fill level (0–100%) for each bin with median filtering |
| **Rain/moisture sensor** | Analog sensor distinguishes wet vs dry organic waste |
| **Metal detector** | Highest-priority detection with memory/flag for metal items |
| **Active buzzer alarm** | Alerts when bins exceed configurable fill threshold (default 55%) |

---

## Hardware Components 🔧

| Component | Purpose |
|-----------|---------|
| ESP32 Dev Board | Main microcontroller |
| 28BYJ-48 Stepper + ULN2003 | Rotating sorter mechanism |
| SG90 / MG90S Servo | Gate control |
| 3× HC-SR04 Ultrasonic | Bin level sensing (wet, dry, metal) |
| FC-51 / TCRT5000 IR sensor | Object detection at input |
| Metal detector module | Metal vs non-metal classification |
| Rain/moisture sensor (analog) | Wet vs dry classification |
| Active buzzer | Fill-level alarm |
| 3× bins | Wet, dry, metal waste |

---

## Pin Connections 🧩

| Component | GPIO | Notes |
|-----------|------|-------|
| **Stepper (ULN2003)** | | |
| IN1 | 4 | Coil 1 |
| IN2 | 2 | Coil 2 |
| IN3 | 18 | Coil 3 |
| IN4 | 19 | Coil 4 |
| **Servo (gate)** | | |
| Signal | 23 | PWM |
| **Sensors** | | |
| IR (main) | 32 | Input, object present = LOW |
| Metal detector | 13 | Input, metal = LOW |
| Rain/moisture (AO) | 35 | Analog, input-only |
| **Buzzer** | 21 | Active-low (LOW = ON) |
| **Ultrasonic (Wet bin)** | TRIG: 26, ECHO: 12 | |
| **Ultrasonic (Dry bin)** | TRIG: 25, ECHO: 14 | |
| **Ultrasonic (Metal bin)** | TRIG: 33, ECHO: 27 | |

---

## Libraries Required 📚

Install via Arduino IDE **Library Manager** or PlatformIO:

| Library | Purpose |
|---------|---------|
| `WiFi` | Built-in ESP32 |
| `HTTPClient` | Built-in ESP32 |
| `ESP32Servo` | Servo control on ESP32 |
| `CheapStepper` | 28BYJ-48 stepper control |

---

## Setup Instructions 🚀

1. **Hardware assembly**
   - Wire components per the pin table above.
   - Ensure stepper and servo have adequate 5 V supply.
   - Mount ultrasonic sensors above each bin, facing down.

2. **Software**
   - Open `Smart_dustbin_2.ino` in Arduino IDE or PlatformIO.
   - Select board: **ESP32 Dev Module** (or your exact board).
   - Select the correct COM port.

3. **Configuration**
   - In the sketch, set your WiFi credentials:
     ```cpp
     const char* ssid     = "YOUR_SSID";
     const char* password = "YOUR_PASSWORD";
     ```
   - Set your ThingSpeak Write API key:
     ```cpp
     String tsApiKey = "YOUR_THINGSPEAK_API_KEY";
     ```
   - Optionally adjust `BIN_DEPTH_CM`, `BIN_EMPTY_CM`, `FULL_THRESHOLD_PCT`, and `RAIN_WET_ON` / `RAIN_WET_OFF` for your setup.

4. **ThingSpeak**
   - Create a channel at [thingspeak.com](https://thingspeak.com).
   - Configure fields: Field 1 = Dry %, Field 2 = Wet %, Field 3 = Metal %, Field 4 = Alarm (0/1).
   - Copy the Write API key into the sketch.

5. **Upload and run**
   - Upload the sketch and open Serial Monitor at 115200 baud.
   - Confirm WiFi connection and ThingSpeak uploads.

---

## How It Works ⚙️

1. **Object detection**  
   An IR sensor at the input detects waste. When it goes LOW, the system starts a sort cycle.

2. **Classification**  
   - Metal detector is checked first; if metal is detected (LOW), waste goes to the metal bin.  
   - Otherwise, the rain/moisture sensor is read. Below the wet threshold → wet bin; above → dry bin.

3. **Sorting**  
   - The stepper rotates to the correct bin (wet = 120° CW, metal = 240° CW; dry = home).  
   - The servo opens the gate for a fixed time to drop the item.  
   - The stepper returns to the home (dry) position.

4. **Level monitoring**  
   Ultrasonic sensors measure distance to waste in each bin. Distance is converted to fill percentage using median-of-3 filtering.

5. **Alarm**  
   If any bin exceeds `FULL_THRESHOLD_PCT`, the alarm flag is set and sent to ThingSpeak. The buzzer beeps 3 times when any bin exceeds the fill threshold.

6. **ThingSpeak upload**  
   Every 15 seconds, the ESP32 sends dry %, wet %, metal %, and alarm to your ThingSpeak channel.

---

## License 📄

MIT
