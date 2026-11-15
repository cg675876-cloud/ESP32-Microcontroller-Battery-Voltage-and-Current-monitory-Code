#[10:16 pm, 14/11/2026] ~Nani"": ESP32-based web monitoring system for solar battery voltage and current with ThingSpeak cloud data logging.
[11:19 pm, 14/11/2026] ~Nani"": ESP32-Based Solar Battery Voltage and Current Monitoring System

Project Overview

This project presents the design and implementation of a web-based remote monitoring system for solar battery voltage and current with data logging.

The system uses an ESP32-WROOM-32 microcontroller to measure battery voltage and current, estimate the battery State of Charge (SOC), control charging through a relay, and transmit monitoring data to the ThingSpeak cloud platform for remote monitoring and historical data logging.

Project Objectives

- To measure solar battery voltage and current using appropriate sensing circuits.
- To process battery measurement data using an ESP32 microcontroller.
- To estimate battery State of Charge (SOC).
- To implement automatic charging control using a relay.
- To provide warning indications using LEDs and a buzzer.
- To transmit battery monitoring data to ThingSpeak for remote monitoring and data logging.

Hardware Components

- ESP32-WROOM-32 development board
- Voltage sensor module
- ACS712 5A current sensor
- Relay module
- Buzzer
- Yellow LED
- Red LED
- Blue LED
- Three-cell lithium-ion battery
- Connecting wires and supporting electronic components

Software and Technologies

- Arduino IDE
- Embedded C/C++
- ESP32 Wi-Fi connectivity
- ThingSpeak IoT cloud platform
- ThingSpeak Arduino library

System Features

- Real-time battery voltage measurement.
- Battery current measurement.
- Estimated State of Charge (SOC).
- Automatic charging control.
- Battery charging and warning indications.
- Wi-Fi connectivity.
- Remote monitoring through ThingSpeak.
- Historical data logging.

ESP32 Pin Configuration

Component| ESP32 Pin
Voltage Sensor| GPIO 34
ACS712 Current Sensor| GPIO 35
Buzzer| GPIO 25
Relay| GPIO 26
Yellow LED| GPIO 32
Red LED| GPIO 33
Blue LED| GPIO 27

ThingSpeak Data Fields

Field| Measurement
Field 1| Battery Voltage
Field 2| Charging/Input Current
Field 3| Discharging/Output Current
Field 4| Battery Power
Field 5| Estimated State of Charge

How the System Works

The voltage and current sensors acquire battery measurements and send their signals to the ESP32 microcontroller.

The ESP32 processes the measurements, estimates the battery State of Charge, and controls the charging relay according to the programmed voltage thresholds.

The monitoring data is transmitted through Wi-Fi to the ThingSpeak cloud platform, where it can be viewed and stored for historical analysis.

Author

Christopher Godwin

B.Eng Electrical and Electronic Engineering

Project: Design and Implementation of a Web-Based Remote Monitoring System for Solar Battery Voltage and Current with Data Logging.
