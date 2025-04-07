# STM32 IoT Server Guardian & LAN Bridge

![STM32](https://img.shields.io/badge/STM32-B--U585I--IOT02A-blue?style=for-the-badge&logo=stmicroelectronics)
![Azure RTOS](https://img.shields.io/badge/Azure_RTOS-NetXDuo-0089D6?style=for-the-badge&logo=microsoftazure)
![MQTT](https://img.shields.io/badge/Protocol-MQTT_Over_TLS-660066?style=for-the-badge)
![C](https://img.shields.io/badge/Language-Embedded_C-A8B9CC?style=for-the-badge&logo=c)
![Hardware](https://img.shields.io/badge/Hardware-Sensors_&_Servos-FF6F00?style=for-the-badge)

An advanced, hardware-in-the-loop IoT Server Guardian built on the **STM32 B-U585I-IOT02A** discovery board. This project serves as a secure LAN Bridge that listens to external commands via public MQTT brokers and executes physical and network-level actions in a local network.

## 🔗 Wi-Fi Foundation & Prerequisites

This project relies on a highly stable Wi-Fi & NetXDuo driver configuration. The low-level network foundation used in this project was developed and documented in a separate repository.

If you are struggling with EMW3080 Wi-Fi connectivity on this board, please refer to my base configuration guide first:

👉 **[STM32-B-U585I-IOT02A_WiFi-Fix](https://github.com/vvojcik/STM32-B-U585I-IOT02A_WiFi-Fix)**

## 🚀 Core Features

1. **Remote Wake-On-LAN (WOL) Bridge:** Receives a secure `WAKE` command via MQTT from anywhere in the world and broadcasts a Magic Packet into the local network to wake up a specific server/PC.
2. **Physical Hard-Reset (Watchdog):** When the target server completely freezes (no SSH/RDP access), the board triggers an SG90 Servo motor to physically press the server's hardware reset button.
3. **Intrusion Detection:** Utilizes an HC-SR04 ultrasonic sensor to monitor the physical space around the server. Triggers an MQTT alarm if unauthorized access is detected.
4. **Visual Dashboard & Status:** Features a UART-driven TFT Dashboard (via STM32F723) and LED matrix animations (X-NUCLEO-LED61A1) to visualize network status, heartbeat, and incoming commands.

## 📂 Modular Architecture

The repository is structured to maintain a clean separation between network middleware, core MCU logic, and external hardware modules:

```text
├── Core/                   # Main application loop and MCU hardware init
├── NetXDuo/App/            # MQTT Client, SNTP time sync, and network logic
├── Drivers/BSP/mx_wifi/    # Patched MXCHIP EMW3080 Wi-Fi drivers
├── Hardware_Modules/       # Peripheral abstraction layer
│   ├── SG90_Servo/         # PWM control for physical button pressing
│   ├── HCSR04_Sonar/       # Distance measurement and interrupt logic
│   ├── LED61A1_Matrix/     # Heartbeat and command visual feedback
│   └── TFT_Dashboard/      # Serial communication for external display
└── docs/                   # Project schematics and terminal logs
```

## ⚙️ Configuration

Before building the project, update the placeholders in the application headers:

1. Set your Wi-Fi credentials in `Core/Inc/mx_wifi_conf.h`.
2. Configure your target PC's MAC address in `NetXDuo/App/app_netxduo.c`.
3. Set a unique `CLIENT_ID_STRING` and define your MQTT topic in `NetXDuo/App/app_netxduo.h` to avoid broker collisions.