# ESP32 Mecanum Rover Web Controller

This repository contains the code for an ESP32-based web controller designed to drive a 4-wheel mecanum rover. It hosts a self-contained Access Point (AP) and serves a MATLAB-style graphical user interface (GUI) directly to your smartphone or computer browser. 

The system handles complex mecanum kinematics and supports a mixed motor driver hardware configuration.

## 🌟 Features

* **Standalone Wi-Fi Access Point:** The ESP32 creates its own network (`ESP32_Mecanum_Rover`), meaning no external Wi-Fi router is required.
* **MATLAB-Style Web Dashboard:** A sleek, responsive web interface tailored for both desktop and mobile devices.
* **Dual Control Modes:**
  * **Kinematic D-Pad:** Omnidirectional control (Forward, Backward, Strafe Left/Right, Rotate Left/Right) using standard mecanum kinematic matrices.
  * **Workspace Control:** Granular, individual RPM/PWM control for each of the four wheels.
* **Live Command Logging:** Built-in web console to track commands and system status in real-time.
* **Mixed Motor Driver Support:** Custom logic to simultaneously drive Cytron MDD3A (Front) and Cytron SmartDriveDuo-10 (Back) motor drivers.

## 🛠️ Hardware Requirements

* 1x ESP32 Microcontroller
* 4x DC Motors with Mecanum Wheels
* 1x Cytron MDD3A Motor Driver (for Front Motors)
* 1x Cytron SmartDriveDuo-10 (MDDS10) Motor Driver (for Back Motors)
* Power supply/Battery suitable for your motors and ESP32

## 🔌 Pin Configuration

The code uses a specific pinout to accommodate the two different motor drivers. Wire your ESP32 to the drivers according to this table:

### Front Motors (Cytron MDD3A - PWM on A & B)
| Motor | ESP32 Pin | Driver Pin | Description |
| :--- | :---: | :---: | :--- |
| **Front Right** | `GPIO 26` | `M1A` | PWM Channel A |
| **Front Right** | `GPIO 27` | `M1B` | PWM Channel B |
| **Front Left** | `GPIO 12` | `M2A` | PWM Channel A |
| **Front Left** | `GPIO 13` | `M2B` | PWM Channel B |

### Back Motors (Cytron SmartDriveDuo-10 - PWM + DIR Mode)
| Motor | ESP32 Pin | Driver Pin | Description |
| :--- | :---: | :---: | :--- |
| **Back Left** | `GPIO 32` | `PWM_BL` | Speed Control |
| **Back Left** | `GPIO 33` | `DIR_BL` | Direction Control |
| **Back Right** | `GPIO 17` | `PWM_BR` | Speed Control |
| **Back Right** | `GPIO 18` | `DIR_BR` | Direction Control |

## 🚀 How to Use

1. **Install Dependencies:** Ensure you have the ESP32 board manager installed in your Arduino IDE. No external libraries are required as it relies on the built-in `<WiFi.h>` and `<WebServer.h>`.
2. **Flash the Code:** Upload `mecanum_wheels.ino` to your ESP32.
3. **Connect to the Rover:**
   * Open the Wi-Fi settings on your phone or computer.
   * Connect to the network: **`ESP32_Mecanum_Rover`**
   * Password: **`password123`**
4. **Access the Dashboard:**
   * Open a web browser and navigate to the ESP32's default AP IP address: **`http://192.168.4.1`** (Check your serial monitor at 115200 baud if the IP differs).

## 🎮 Web Interface Overview

* **Master PWM Speed:** Sets the global speed (0-255) for the D-Pad commands.
* **Kinematic Controller (D-Pad):** Touch-friendly buttons for full omnidirectional movement. Automatically stops motors when the button is released.
* **Workspace (Individual RPM):** Input specific PWM values (-255 to 255) for FL, FR, BL, and BR motors. Click "Send to Workspace" to execute.
* **Command Window:** A read-only logger mimicking a MATLAB console to show sent commands and debugging info.

## 📝 Future Improvements / Notes
*(Note: As the repository is named `imu_controlled_mecanumbot`, future commits can integrate MPU6050/BNO055 logic here for closed-loop orientation control and stabilization).*
