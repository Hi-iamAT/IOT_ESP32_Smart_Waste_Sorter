<div align="center">

<img src="https://readme-typing-svg.herokuapp.com?font=Fira+Code&size=32&pause=1000&color=00C853&center=true&vCenter=true&width=600&lines=♻️+ESP32+Smart+Waste+Sorter;IoT+Waste+Classification+System;Wet+%7C+Dry+%7C+Metal+Sorting" alt="Typing SVG" />

<br/>

<img src="https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge&logo=espressif&logoColor=white"/>
<img src="https://img.shields.io/badge/Framework-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white"/>
<img src="https://img.shields.io/badge/Cloud-ThingSpeak-orange?style=for-the-badge&logoColor=white"/>
<img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge"/>
<img src="https://img.shields.io/badge/Language-C%2B%2B-red?style=for-the-badge&logo=cplusplus&logoColor=white"/>

<br/><br/>

<img src="https://img.shields.io/badge/WiFi-Enabled-brightgreen?style=flat-square&logo=wifi&logoColor=white"/>
<img src="https://img.shields.io/badge/Sensors-Ultrasonic%20%7C%20IR%20%7C%20Metal%20%7C%20Rain-blue?style=flat-square"/>
<img src="https://img.shields.io/badge/Actuators-Stepper%20%7C%20Servo-orange?style=flat-square"/>
<img src="https://img.shields.io/badge/Alert-Buzzer%20Alarm-red?style=flat-square"/>
<img src="https://img.shields.io/badge/Status-Active-success?style=flat-square"/>

<br/><br/>

> ### 🌍 *Automatically classifies and routes waste into wet, dry, and metal bins*
> ### ☁️ *Real-time cloud monitoring via ThingSpeak*

<br/>

---

</div>

## 📝 Project Description

This project implements an **autonomous waste sorting mechanism** that detects
incoming waste, classifies it using metal detection and moisture sensing,
and directs it to the appropriate bin using a stepper motor and
servo-controlled gate.

Bin fill levels are monitored in real time with ultrasonic sensors, and data
is uploaded to **ThingSpeak** for remote monitoring and alerting.

<details>
<summary>🔍 <b>Click to read more...</b></summary>
<br/>

The system is designed for smart city and campus waste management applications.
It eliminates the need for manual waste sorting by automatically detecting
whether waste is wet (organic), dry (recyclable), or metal — and routing it
to the correct bin without any human intervention.

The ESP32 microcontroller handles all logic, WiFi connectivity, sensor reading,
and motor control simultaneously. ThingSpeak provides a free IoT cloud dashboard
for monitoring bin levels remotely from any device.

</details>

---

## 🌟 Features

| Feature | Description |
|---------|-------------|
| 📶 **WiFi + ThingSpeak IoT** | Uploads bin levels and alarm status every 15 seconds |
| 🗑️ **Three-bin sorting** | Wet, dry, and metal waste with automatic classification |
| ⚙️ **Stepper motor sorter** | Rotates to target bin (120° wet, 240° metal) |
| 🚪 **Servo gate** | Opens/closes to drop waste into selected bin |
| 📡 **Ultrasonic bin sensors** | Fill level (0–100%) with median-of-3 filtering |
| 💧 **Rain/moisture sensor** | Analog sensor distinguishes wet vs dry waste |
| 🔩 **Metal detector** | Highest-priority with memory flag system |
| 🔔 **Active buzzer alarm** | Beeps 3× when bin exceeds 55% fill threshold |
| 🔄 **WiFi auto-reconnect** | Reconnects automatically if WiFi drops |
| 🧠 **Smart metal memory** | Remembers metal detection for 2 seconds after sensor passes |

---

## 🏗️ System Architecture
```
                    ┌─────────────┐
    Waste Input ──► │  IR Sensor  │
                    └──────┬──────┘
                           │ object detected
                    ┌──────▼──────┐
                    │  ESP32 MCU  │◄──── WiFi ────► ThingSpeak ☁️
                    └──────┬──────┘
              ┌────────────┼────────────┐
              ▼            ▼            ▼
         Metal Det.   Rain Sensor   Ultrasonic x3
         (GPIO 13)    (GPIO 35)     (fill level)
              │            │
              ▼            ▼
         ┌──────────────────────┐
         │    Stepper Motor     │
         │    (Route Selector)  │
         │   0°=DRY 120°=WET    │
         │      240°=METAL      │
         └───────────┬──────────┘
                     │
         ┌───────────▼──────────┐
         │     Servo Gate       │
         │  (opens to drop item)│
         └───────────┬──────────┘
               ┌─────┼─────┐
               ▼     ▼     ▼
            💧 WET 📦 DRY 🔩 METAL
```

---

## 🔧 Hardware Components

| # | Component | Purpose |
|---|-----------|---------|
| 1 | 🧠 ESP32 Dev Board | Main microcontroller |
| 2 | ⚙️ 28BYJ-48 Stepper + ULN2003 | Rotating sorter mechanism |
| 3 | 🔄 SG90 / MG90S Servo | Gate control |
| 4 | 📡 3× HC-SR04 Ultrasonic | Bin level sensing |
| 5 | 👁️ FC-51 / TCRT5000 IR sensor | Object detection at input |
| 6 | 🔩 Metal detector module | Metal vs non-metal classification |
| 7 | 💧 Rain/moisture sensor (analog) | Wet vs dry classification |
| 8 | 🔔 Active buzzer | Fill-level alarm |
| 9 | 🗑️ 3× bins | Wet, dry, metal waste containers |
| 10 | 🔋 9V battery + connector | Power for metal detector |
| 11 | 💻 USB / Power bank (5V) | Power for ESP32 + all sensors |

---

## 🧩 Pin Connections

| Component | GPIO | Notes |
|-----------|------|-------|
| **⚙️ Stepper IN1** | 4 | Coil 1 |
| **⚙️ Stepper IN2** | 2 | Coil 2 |
| **⚙️ Stepper IN3** | 18 | Coil 3 |
| **⚙️ Stepper IN4** | 19 | Coil 4 |
| **🔄 Servo Signal** | 23 | PWM output |
| **👁️ IR Sensor** | 32 | Object present = LOW |
| **🔩 Metal detector** | 13 | Metal detected = LOW |
| **💧 Rain/moisture AO** | 35 | Analog input-only pin |
| **🔔 Buzzer** | 21 | Active-low (LOW = ON) |
| **📡 Ultrasonic Wet** | TRIG: 26, ECHO: 12 | Wet bin level |
| **📡 Ultrasonic Dry** | TRIG: 25, ECHO: 14 | Dry bin level |
| **📡 Ultrasonic Metal** | TRIG: 33, ECHO: 27 | Metal bin level |

---

## ⚡ Power Setup
```
╔══════════════════════════════════════╗
║   Laptop USB / Power Bank (5V)       ║
║         │                            ║
║         └──► ESP32 (via USB)         ║
║              │                       ║
║              ├──► Stepper + ULN2003  ║
║              ├──► Servo              ║
║              ├──► IR Sensor          ║
║              ├──► Ultrasonic x3      ║
║              ├──► Rain Sensor        ║
║              └──► Buzzer             ║
╠══════════════════════════════════════╣
║   9V Battery + Connector             ║
║         │                            ║
║         └──► Metal Detector VCC      ║
║              Metal Detector GND      ║
║              └──► ESP32 GND ✅       ║
╚══════════════════════════════════════╝
```

> ⚠️ **Critical:** Metal detector GND and ESP32 GND **must share common ground**
> for correct signal reading on GPIO 13.

---

## 📚 Libraries Required

Install via Arduino IDE **Library Manager** (`Sketch` → `Include Library` → `Manage Libraries`):

| Library | Install Name | Purpose |
|---------|-------------|---------|
| `WiFi` | *(Built-in ESP32)* | WiFi connectivity |
| `HTTPClient` | *(Built-in ESP32)* | HTTP requests to ThingSpeak |
| `ESP32Servo` | `ESP32Servo` | Servo control on ESP32 |
| `CheapStepper` | `CheapStepper` | 28BYJ-48 stepper control |

---

## 🚀 Setup Instructions

<details>
<summary>1️⃣ <b>Hardware Assembly</b></summary>
<br/>

- Wire all components per the pin table above
- Connect metal detector **positive** to 9V battery
- Connect metal detector **GND** to ESP32 GND (common ground!)
- Connect everything else to USB/power bank via ESP32
- Mount ultrasonic sensors **above each bin facing straight down**
- Ensure stepper and servo have adequate **5V supply**

</details>

<details>
<summary>2️⃣ <b>Software Setup</b></summary>
<br/>

- Open `Smart_dustbin_2.ino` in **Arduino IDE**
- Go to `Tools` → `Board` → Select **ESP32 Dev Module**
- Go to `Tools` → `Port` → Select correct **COM port**
- Install all required libraries listed above

</details>

<details>
<summary>3️⃣ <b>Configuration</b></summary>
<br/>

Set your WiFi credentials:
```cpp
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
```

Set your ThingSpeak Write API key:
```cpp
String tsApiKey = "YOUR_THINGSPEAK_API_KEY";
```

Tune for your physical bin size:
```cpp
const int BIN_EMPTY_CM       = 7;    // distance (cm) when bin is empty
const int FULL_THRESHOLD_PCT = 55;   // alarm trigger at 55% full
const int RAIN_WET_ON        = 3000; // moisture wet threshold
const int RAIN_WET_OFF       = 3200; // moisture dry threshold
```

</details>

<details>
<summary>4️⃣ <b>ThingSpeak Setup</b></summary>
<br/>

- Create a free channel at [thingspeak.com](https://thingspeak.com)
- Configure fields:

| Field | Data | Unit |
|-------|------|------|
| Field 1 | Dry bin level | % |
| Field 2 | Wet bin level | % |
| Field 3 | Metal bin level | % |
| Field 4 | Alarm status | 0 or 1 |

- Copy the **Write API Key** into the sketch

</details>

<details>
<summary>5️⃣ <b>Upload & Run</b></summary>
<br/>

- Click **Upload** in Arduino IDE
- Open **Serial Monitor** at `115200 baud`
- Confirm WiFi connection message
- Confirm ThingSpeak response code **200**
- Test by placing objects in the bin input

</details>

---

## ⚙️ How It Works

### 1. 👁️ Object Detection
IR sensor at bin input detects waste. When signal goes **LOW** → sort cycle begins.

### 2. 🧠 Smart Classification
```
Object detected by IR sensor
           │
           ▼
  Metal flag set within          ──YES──► 🔩 METAL bin
  last 2 seconds?
           │ NO
           ▼
  Rain sensor reading             ──YES──► 💧 WET bin
  below wet threshold?
           │ NO
           ▼
                                           📦 DRY bin
```

> 💡 **Metal Memory System:** The metal sensor is at the **top** of the bin,
> IR sensor at the **bottom**. When metal object passes the top sensor,
> a flag is set for 2 seconds — so by the time IR triggers at the bottom,
> metal is still correctly identified.

### 3. ⚙️ Sorting Sequence
```
1. Stepper rotates CW to target position
   - Dry  →   0° (home)
   - Wet  → 120°
   - Metal→ 240°
2. Servo opens gate (1.2 seconds)
3. Item drops into correct bin
4. Servo closes gate
5. Stepper returns CCW to home
```

### 4. 📡 Bin Level Monitoring
- 3 ultrasonic readings taken per bin
- **Median of 3** used to eliminate false readings
- Distance converted to fill percentage

### 5. 🔔 Alarm System
```
Every 15 seconds:
   Read all 3 bin levels
         │
         ▼
   Any bin ≥ 55%? ──YES──► Buzzer beeps 3× 🔔
         │                  Upload alarm=1 to ThingSpeak
         │ NO
         ▼
   Upload alarm=0 to ThingSpeak
```

### 6. ☁️ ThingSpeak Upload
Every **15 seconds**, ESP32 sends to cloud:
- 📦 Dry bin fill %
- 💧 Wet bin fill %
- 🔩 Metal bin fill %
- 🚨 Alarm status (0 = OK, 1 = Full)

---

## 🐛 Troubleshooting

<details>
<summary>🔍 <b>Click to expand troubleshooting guide</b></summary>
<br/>

| Problem | Likely Cause | Fix |
|---------|-------------|-----|
| Metal not sorting | Flag timing issue | Check GPIO 13 wiring and common GND |
| Stepper wrong position | Lost home position | Power cycle to rehome stepper |
| Bins always 0% | Ultrasonic not working | Check TRIG/ECHO pin connections |
| ThingSpeak error -1 | WiFi disconnected | Check SSID and password |
| Buzzer always ON | Active-low confusion | Verify GPIO 21, check buzzer type |
| Rain always wet | Threshold too high | Lower `RAIN_WET_ON` value in code |
| Servo not moving | Wrong PWM range | Adjust `SERVO_MIN_US`/`SERVO_MAX_US` |
| WiFi won't connect | Wrong credentials | Double check SSID/password in code |

</details>

---

## 📄 License

This project is released under the **MIT License** — free to use, modify and share!

See [LICENSE](LICENSE) for full details.

---

## 🙏 Credits

- **ESP32 Framework** — [Espressif Systems](https://www.espressif.com)
- **ThingSpeak IoT Platform** — [MathWorks ThingSpeak](https://thingspeak.com)
- **CheapStepper Library** — [tyhenry/CheapStepper](https://github.com/tyhenry/CheapStepper)
- **ESP32Servo Library** — [madhephaestus/ESP32Servo](https://github.com/madhephaestus/ESP32Servo)
- **Shields.io Badges** — [shields.io](https://shields.io)
- **Built with** — Arduino framework + ESP32

---

<div align="center">

<img src="https://readme-typing-svg.herokuapp.com?font=Fira+Code&size=16&pause=1000&color=00C853&center=true&vCenter=true&width=500&lines=Made+with+❤️+by+Atalauddin;University+IoT+Project+🎓;Bangladesh+%F0%9F%87%A7%F0%9F%87%A9" alt="Footer" />

<br/>

⭐ **Star this repo if you found it helpful!** ⭐

<br/>

![GitHub repo size](https://img.shields.io/github/repo-size/Hi-iamAT/IOT_ESP32-Smart-Waste-Sorter?style=flat-square)
![GitHub last commit](https://img.shields.io/github/last-commit/Hi-iamAT/IOT_ESP32-Smart-Waste-Sorter?style=flat-square)
![GitHub stars](https://img.shields.io/github/stars/Hi-iamAT/IOT_ESP32-Smart-Waste-Sorter?style=flat-square)

</div>