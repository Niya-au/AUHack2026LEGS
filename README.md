# 🦵 Drink-A-Leg — AUHack 2026

> A 3D-moulded leg you can drink from. Yes, really.

Drink-A-Leg is a novelty beverage challenge device built for AUHack 2026. A custom 3D-moulded leg is fitted with internal tubing so any beverage can be consumed through it. Users register via a mobile web app before taking their turn, and onboard sensors automatically log how long they took to finish the leg and their average flow rate — all synced to a live Supabase database.

---

## 🗂️ Repository Structure

```
AUHack2026LEGS/
│
├── auHackLegRegistration.html   # Mobile web app for user registration
├── regPageTrims.css             # Styles for the registration app
│
├── ESP32_final.ino              # Main microcontroller code (reads sensors, posts to DB)
├── ESP32_water_level/           # Test code for elapsed time & average flow rate
│
├── examples/                   # HTML interface test code
└── espSupabaseExample/         # WiFi + Supabase connection test code for ESP32
```

---

## 🛠️ Tech Stack

| Layer | Technology |
|---|---|
| Microcontroller | ESP32 DevKit |
| Sensors | Water level sensor, water flow rate sensor |
| Mobile App | HTML + CSS (mobile web) |
| Backend / Database | Supabase |

---

## ⚙️ Setup

### 1. Database (Supabase)

Create a new Supabase project and add a table (e.g. `legs`) with the following columns:

| Column | Type | Description |
|---|---|---|
| `id` | uuid (PK) | Auto-generated |
| `name` | text | Registered user's name |
| `time_seconds` | float | Time taken to finish the leg |
| `avg_flow_rate` | float | Average flow rate (L/min) |
| `created_at` | timestamp | Auto-generated |

Copy your **Supabase URL** and **anon public key** — you'll need these for both the web app and the ESP32.

---

### 2. Mobile Registration App

Open `auHackLegRegistration.html` in any browser (works best on mobile). Before using:

- Set your **Supabase URL** and **API key** in the script section of `auHackLegRegistration.html`.
- The app allows a new user to enter their name and create a database entry at the press of a button.
- No build step needed — it runs directly in the browser.

---

### 3. ESP32 Firmware

#### Prerequisites

Install the following libraries via the Arduino Library Manager:

- `WiFi` (built-in)
- `HTTPClient` (built-in)
- `ArduinoJson`

#### Configuration

In `ESP32_final.ino`, update the following constants:

```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* supabaseUrl  = "https://your-project.supabase.co";
const char* supabaseKey  = "your-anon-public-key";
```

#### Flashing

1. Open `ESP32_final.ino` in the Arduino IDE.
2. Select **ESP32 Dev Module** under Tools → Board.
3. Select the correct COM port.
4. Upload.

---

## 🔌 Hardware Wiring

> All sensors connect to the ESP32 DevKit via the following pin assignments. Adjust `ESP32_final.ino` if you use different pins.

```
ESP32 DevKit
│
├── 3.3V  ──────────── VCC (both sensors)
├── GND   ──────────── GND (both sensors)
│
├── GPIO 34 (INPUT) ── Water Level Sensor (signal/analog out)
└── GPIO 13 (INPUT) ── Water Flow Sensor (pulse/digital out)
```

**Water Level Sensor** — reads analog voltage to detect when the leg is empty (i.e. the drink is finished). The ESP32 polls this on a set interval to determine the end of a session.

**Water Flow Sensor** — outputs pulses proportional to flow rate. The ESP32 counts pulses over time to calculate average flow rate in L/min.

> ⚠️ Ensure both sensors are rated for 3.3V logic levels, or use a logic level shifter if using 5V sensors.

---

## 🧪 Testing

- **`ESP32_water_level/`** — standalone sketch to verify elapsed time tracking and flow rate calculation on the hardware before connecting to Supabase.
- **`examples/`** — static HTML pages to test the registration UI without a live backend.
- **`espSupabaseExample/`** — minimal ESP32 sketch to confirm WiFi connectivity and Supabase POST requests are working correctly.

---

## 👥 Team

Built with chaos and questionable hydration at **AUHack 2026**.
