Project Report: https://tinyurl.com/37w5h357

## Malkani Bike Computer Project

A custom built, indoor bicycle computer designed to provide real-time cycling metrics for indoor base training. This system was developed as an Independent Study Project for TEJ3M (RSGC ACES).

If you have any questions/suggestions at all PLEASE EMAIL: zmalkani@rsgc.on.ca and I will respond as soon as possible.
---

### Overview

This project is a homemade indoor bike computer built to track essential riding data during stationary cycling sessions. It offers an extremely cost-effective, custom alternative to commercial displays, with a focus on accuracy for indoor training environments. This project on breadboard/protoboard can provide Cadence and Speed measurements for less than $16 Canadian.

### Features

* **Real-time Speed Tracking:** Calculates speed based on wheel rotation and sensor input.
* **Distance & Trip Odometer:** Tracks the total distance "traveled" during a workout session.
* **Indoor Optimized:** Specifically calibrated for the dimensions and resistance profiles of an indoor stationary bike.
* **Custom Interface:** A clean LCD UI made for high readability while exercising.
* **C++ Foundation:** Programmed in C++, making it adaptable for various microcontrollers (ESP32, 328P, etc.).

### Hardware Requirements

* **Microcontroller:** (e.g., Arduino Uno, Nano, or ESP32)
* **Sensor:** Reed switch + magnet (for wheel rotation tracking)
* **Display:** (e.g., OLED, LCD Screen, or Serial)
* **Power:** 5V USB power supply or battery
* **Magnets:** Attached to the flywheel or wheel spokes

### Installation and Setup

1. **Clone the repository:**
`git clone https://github.com/zmalkani/Malkani-Bike-Computer-Project.git`
2. **Open in IDE:** Open the project folder in the Arduino IDE or PlatformIO.
3. **Configure Pins:** Ensure the pin definitions in the code match your specific hardware wiring.
4. **Upload:** Flash the code to your microcontroller.

### How it Works

The computer functions by measuring the time interval between pulses from a magnetic sensor mounted near the bike's flywheel. Every time the magnet passes the sensor, the code calculates the RPM (Rotations Per Minute) and converts it into speed and distance based on the specific wheel circumference configured in the software.
