<div align="center">

# ♻️ ESP32 Smart Waste Sorter 🤖

### An IoT-enabled autonomous waste sorting system

<p>
  <img src="https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge&logo=espressif&logoColor=white"/>
  <img src="https://img.shields.io/badge/Framework-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white"/>
  <img src="https://img.shields.io/badge/Cloud-ThingSpeak-orange?style=for-the-badge&logoColor=white"/>
  <img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Language-C%2B%2B-red?style=for-the-badge&logo=cplusplus&logoColor=white"/>
  <img src="https://img.shields.io/badge/Bins-Wet%20%7C%20Dry%20%7C%20Metal-purple?style=for-the-badge"/>
</p>

<p>
  <img src="https://img.shields.io/badge/WiFi-Enabled-brightgreen?style=flat-square&logo=wifi"/>
  <img src="https://img.shields.io/badge/Sensors-Ultrasonic%20%7C%20IR%20%7C%20Metal%20%7C%20Rain-blue?style=flat-square"/>
  <img src="https://img.shields.io/badge/Actuators-Stepper%20%7C%20Servo-orange?style=flat-square"/>
  <img src="https://img.shields.io/badge/Alert-Buzzer%20Alarm-red?style=flat-square"/>
</p>

> **Automatically classifies and routes waste into wet, dry, and metal bins
> using ESP32 with real-time cloud monitoring via ThingSpeak.**

</div>

---

## 📝 Project Description

This project implements an autonomous waste sorting mechanism that detects
incoming waste, classifies it using metal detection and moisture sensing,
and directs it to the appropriate bin using a stepper motor and
servo-controlled gate. Bin fill levels are monitored in real time with
ultrasonic sensors, and data is uploaded to ThingSpeak for remote
monitoring and alerting.

---

## 🌟 Features

| Feature | Description |
|---------|-------------|
| 📶 **WiFi + ThingSpeak IoT** | Uploads bin levels and alarm status to ThingSpeak every 15 seconds |
| 🗑️ **Three-bin sorting** | Wet, dry, and metal waste bins with automatic classification |
| ⚙️ **Stepper motor sorter** | Rotates to align with the target bin (120° wet, 240° metal) |
| 🚪 **Servo gate** | Opens/closes to drop waste into the selected bin |
| 📡 **Ultrasonic bin sensors** | Monitors fill level (0–100%) for each bin with median filtering |
| 💧 **Rain/moisture sensor** | Analog sensor distinguishes wet vs dry organic waste |
| 🔩 **Metal detector** | Highest-priority detection with memory/flag for metal items |
| 🔔 **Active buzzer alarm** | Alerts when bins exceed configurable fill threshold (default 55%) |
| 🔄 **WiFi auto-reconnect** | Automatically reconnects if WiFi drops during operation |

---

## 🏗️ System Architecture
```
                    ┌─────────────┐
    Waste Input ──► │  IR Sensor  │
                    └──────┬──────┘
                           │ detected
                    ┌──────▼──────┐
                    │  ESP32 MCU  │◄──── WiFi ──── ThingSpeak ☁️
                    └──────┬──────┘
              ┌────────────┼────────────┐
              ▼            ▼            ▼
         Metal Det.   Rain Sensor   Ultrasonic x3
              │            │
              ▼            ▼
         ┌─────────────────────┐
         │   Stepper Motor     │
         │   (Route Selector)  │
         └──────────┬──────────┘
                    │
         ┌──────────▼──────────┐
         │    Servo Gate       │
         └──────────┬──────────┘
            ┌───────┼───────┐
            ▼       ▼       ▼
         💧 WET   📦 DRY  🔩 METAL
```

---

## 🔧 Hardware Components

| Component | Purpose |
|-----------|---------|
| 🧠 ESP32 Dev Board | Main microcontroller |
| ⚙️ 28BYJ-48 Stepper + ULN2003 | Rotating sorter mechanism |
| 🔄 SG90 / MG90S Servo | Gate control |
| 📡 3× HC-SR04 Ultrasonic | Bin level sensing (wet, dry, metal) |
| 👁️ FC-51 / TCRT5000 IR sensor | Object detection at input |
| 🔩 Metal detector module | Metal vs non-metal classification |
| 💧 Rain/moisture sensor (analog) | Wet vs dry classification |
| 🔔 Active buzzer | Fill-level alarm |
| 🗑️ 3× bins | Wet, dry, metal waste |
| 🔋 9V battery | Power for metal detector |
| 💻 USB / Power bank | Power for ESP32 + sensors |

---

## 🧩 Pin Connections

| Component | GPIO | Notes |
|-----------|------|-------|
| **⚙️ Stepper (ULN2003)** | | |
| IN1 | 4 | Coil 1 |
| IN2 | 2 | Coil 2 |
| IN3 | 18 | Coil 3 |
| IN4 | 19 | Coil 4 |
| **🔄 Servo (gate)** | | |
| Signal | 23 | PWM |
| **👁️ IR Sensor** | 32 | Object present = LOW |
| **🔩 Metal detector** | 13 | Metal detected = LOW |
| **💧 Rain/moisture (AO)** | 35 | Analog input-only |
| **🔔 Buzzer** | 21 | Active-low (LOW = ON) |
| **📡 Ultrasonic — Wet bin** | TRIG: 26, ECHO: 12 | |
| **📡 Ultrasonic — Dry bin** | TRIG: 25, ECHO: 14 | |
| **📡 Ultrasonic — Metal bin** | TRIG: 33, ECHO: 27 | |

---

## ⚡ Power Setup
```
Laptop USB / Power Bank (5V)
        │
        └──► ESP32 + Stepper + Servo + All Sensors

9V Battery + Connector
        │
        └──► Metal Detector only
             (GND must be common with ESP32!)
```

> ⚠️ **Important:** Metal detector GND and ESP32 GND must be connected
> together (common ground) for correct signal reading.

---

## 📚 Libraries Required

Install via Arduino IDE **Library Manager**:

| Library | Install Name | Purpose |
|---------|-------------|---------|
| `WiFi` | Built-in ESP32 | WiFi connectivity |
| `HTTPClient` | Built-in ESP32 | HTTP requests to ThingSpeak |
| `ESP32Servo` | `ESP32Servo` | Servo control on ESP32 |
| `CheapStepper` | `CheapStepper` | 28BYJ-48 stepper control |
| `ArduinoJson` | `ArduinoJson` | JSON parsing (optional) |

---

## 🚀 Setup Instructions

### 1️⃣ Hardware Assembly
- Wire all components per the pin table above
- Connect metal detector to 9V battery
- Connect everything else to USB/power bank via ESP32
- Mount ultrasonic sensors **above each bin facing down**
- Ensure **common GND** between 9V and USB supply

### 2️⃣ Software
- Open `Smart_dustbin_2.ino` in **Arduino IDE**
- Select board: **ESP32 Dev Module**
- Select the correct **COM port**

### 3️⃣ Configuration
Set your WiFi credentials in the sketch:
```cpp
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
```

Set your ThingSpeak Write API key:
```cpp
String tsApiKey = "YOUR_THINGSPEAK_API_KEY";
```

Tune these values for your physical bin size:
```cpp
const int BIN_EMPTY_CM       = 7;   // distance when bin is empty
const int FULL_THRESHOLD_PCT = 55;  // alarm at 55% full
const int RAIN_WET_ON        = 3000; // moisture threshold
```

### 4️⃣ ThingSpeak Setup
- Create a free channel at [thingspeak.com](https://thingspeak.com)
- Configure fields:

| Field | Data |
|-------|------|
| Field 1 | Dry bin % |
| Field 2 | Wet bin % |
| Field 3 | Metal bin % |
| Field 4 | Alarm (0 or 1) |

### 5️⃣ Upload & Run
- Upload sketch and open **Serial Monitor at 115200 baud**
- Confirm WiFi connection and ThingSpeak response code **200**

---

## ⚙️ How It Works

### 1. 👁️ Object Detection
An IR sensor at the bin input detects waste. When it goes LOW, the system starts a sort cycle.

### 2. 🧠 Classification
```
Object detected by IR
        │
        ▼
Metal sensor flag set? ──YES──► Route to METAL bin 🔩
        │NO
        ▼
Rain sensor wet? ────────YES──► Route to WET bin 💧
        │NO
        ▼
                         Route to DRY bin 📦
```

### 3. ⚙️ Sorting
- Stepper rotates to target bin position
- Servo opens gate → item drops
- Stepper returns to home (dry) position

### 4. 📡 Level Monitoring
Ultrasonic sensors take 3 readings per bin and use **median filtering** to get accurate fill percentage.

### 5. 🔔 Alarm
When any bin exceeds **55% full:**
- Buzzer beeps **3 times**
- Alarm flag **uploaded to ThingSpeak**

### 6. ☁️ ThingSpeak Upload
Every **15 seconds**, ESP32 sends:
- Dry bin fill %
- Wet bin fill %
- Metal bin fill %
- Alarm status (0/1)

---

## 📊 ThingSpeak Dashboard

Monitor your bins live at:
```
https://thingspeak.mathworks.com/channels/3245840
```

---

## 🐛 Troubleshooting

| Problem | Likely Cause | Fix |
|---------|-------------|-----|
| Metal not sorting correctly | Metal sensor flag timing | Check metal sensor wiring to GPIO 13 |
| Stepper goes wrong direction | Wrong home position | Power cycle to rehome |
| Bins always show 0% | Ultrasonic wiring issue | Check TRIG/ECHO pins |
| ThingSpeak shows -1 | WiFi not connected | Check SSID/password |
| Buzzer always ON | Active-low logic | Check buzzer wiring to GPIO 21 |
| Rain always wet | Threshold too high | Lower `RAIN_WET_ON` value |

---

## 📄 License

MIT — feel free to use, modify and share!

---

<div align="center">

**Made with ❤️ using ESP32 + Arduino**

⭐ Star this repo if you found it helpful!

</div>