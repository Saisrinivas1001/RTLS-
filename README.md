# Multi-Tag Localization System (ESP + BLE)

## Overview

This project implements a multi-tag indoor localization system using ESP-based anchor nodes and BLE advertising tags. The system tracks multiple BLE tags in real time by collecting signal data across multiple anchors.

The architecture consists of:

* BLE Tags broadcasting unique identifiers
* Anchor nodes scanning and collecting BLE signal data
* A Processing Unit aggregating data for further analysis

---

## System Architecture

```
          +-------------+
          |   TAGS      |
          | (BLE Nodes) |
          +------+------+ 
                 |
        BLE Advertising
                 |
   +------+------+------+------+------+
   | Anchor 1 | Anchor 2 | Anchor 3 | Anchor 4 |
   +------+------+------+------+------+
                 |
         Data Transmission
                 |
        +------------------+
        | Processing Unit  |
        |   (ESP_PU)       |
        +------------------+
```

---

## Project Structure

```
Project_Approach_2_multitags/
│
├── anchor_1/        # Code for Anchor Node 1
├── anchor_2/        # Code for Anchor Node 2
├── anchor_3/        # Code for Anchor Node 3
├── anchor_4/        # Code for Anchor Node 4
│
├── TAG/             # BLE Tag implementation
├── ESP_PU/          # Processing Unit logic
│
└── pppp/            # Additional or experimental scripts
```

---

## Components

### BLE Tag (`TAG/TAG.ino`)

* Broadcasts a unique TAG_ID
* Uses BLE advertising
* Designed to be lightweight and power-efficient

Example:

```cpp
#define TAG_ID "TAG_1"
```

---

### Anchors (`anchor_*/anchor_*.ino`)

* Scan nearby BLE devices
* Capture RSSI (signal strength)
* Forward collected data to the processing unit

---

### Processing Unit (`ESP_PU/ESP_PU.ino`)

* Collects data from all anchors
* Performs:

  * Data aggregation
  * Tag identification
  * Optional position estimation logic

---

## Getting Started

### Hardware Requirements

* ESP32 boards (minimum 5 recommended)

  * 4 Anchors
  * 1 Processing Unit
* Power supply (USB or battery)
* BLE-enabled environment

---

### Setup Instructions

#### Step 1: Upload Tag Code

* Open `TAG/TAG.ino` in Arduino IDE
* Modify:

```cpp
#define TAG_ID "TAG_X"
```

* Upload to ESP32 used as a tag

---

#### Step 2: Upload Anchor Codes

* Flash each anchor directory (`anchor_1`, `anchor_2`, etc.) to separate ESP32 boards

---

#### Step 3: Upload Processing Unit Code

* Flash `ESP_PU.ino` to the central ESP32

---

### Serial Monitoring

* Use Arduino Serial Monitor (115200 baud)
* Monitor:

  * Tag detection logs
  * RSSI values
  * Data transmission status

---

## Working Principle

1. Tags continuously broadcast BLE signals
2. Anchors detect nearby tags through scanning
3. Each anchor records:

   * Tag ID
   * RSSI (signal strength)
4. Data is sent to the Processing Unit
5. The Processing Unit aggregates and processes the data

---

## Applications

* Indoor positioning systems
* Asset tracking
* Smart warehouse monitoring
* Proximity detection

---

## Future Improvements

* Trilateration-based positioning
* Hybrid BLE + WiFi localization
* Cloud integration for monitoring
* Mobile application interface
* Power optimization for tags

---

## Notes

* BLE RSSI values can be noisy; filtering techniques are recommended
* Anchor placement significantly affects localization accuracy
* Ensure each tag has a unique TAG_ID

---

## Contributing

Contributions are welcome. Possible areas:

* Localization algorithms
* Visualization tools
* Communication optimization

---

## License

Add an appropriate open-source license (MIT recommended) if not already included.

---

## Author

Developed as part of an embedded systems / IoT localization project.
