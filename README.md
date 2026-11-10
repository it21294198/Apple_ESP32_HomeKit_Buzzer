# ESP32 Super Mini – Apple HomeKit Buzzer Controller

A DIY HomeKit-compatible buzzer controller built using the **ESP32-C3 Super Mini** and [HomeSpan](https://github.com/HomeSpan/HomeSpan).

This project allows you to control a passive buzzer through Apple's Home app or a physical push button connected to the ESP32.

## Features

- Apple HomeKit integration using HomeSpan.
- Control a passive buzzer through the Apple Home app.
- Physical push-button control.
- Hardware PWM tone generation using ESP32 LEDC.
- Automatic Wi-Fi reconnection after restart.
- Persistent HomeKit pairing information.
- No need to re-pair after a normal power cycle.
- Buzzer defaults to OFF after restarting.

## Hardware Requirements

| Component | Quantity |
|---|---:|
| ESP32-C3 Super Mini | 1 |
| Passive Piezo Buzzer | 1 |
| Push Button | 1 |
| Jumper Wires | As required |
| USB Power Supply | 1 |

## Hardware Connections

### Pin Configuration

| Component | ESP32 GPIO |
|---|---|
| Passive Buzzer (+) | GPIO 7 |
| Passive Buzzer (-) | GND |
| Push Button Terminal 1 | GPIO 6 |
| Push Button Terminal 2 | GND |

The push button uses the ESP32 internal pull-up resistor (`INPUT_PULLUP`).

**Note:** A small piezo buzzer can generally be driven directly. Use a transistor or MOSFET driver for buzzers requiring more current than the GPIO can safely provide.

## Software Requirements

- Arduino IDE
- ESP32 Arduino Core 3.x
- HomeSpan Library 2.1.8 or compatible version
- Apple iPhone with the Apple Home app
- 2.4 GHz Wi-Fi network

### Install HomeSpan

1. Open Arduino IDE.
2. Navigate to Library Manager.
3. Search for `HomeSpan`.
4. Install the library.

### Arduino IDE Configuration

| Setting | Configuration |
|---|---|
| Board | ESP32C3 Dev Module |
| Flash Size | 4MB (32Mb) |
| Partition Scheme | Minimal SPIFFS or suitable large application partition |
| CPU Frequency | 160 MHz |
| USB CDC On Boot | Enabled |
| Core Debug Level | None |

The application partition must be larger than the compiled firmware size.

1. Configure Wi-Fi using Serial Monitor
You don't need to change your buzzer code.
1. Open Arduino IDE Serial Monitor.
2. Set baud rate to 115200.
3. Set line ending to Newline.
4. Type:
W


5. Press Enter.

HomeSpan should scan for nearby Wi-Fi networks and ask you to select your network and enter its password.

This stores your Wi-Fi credentials in ESP32 flash memory. 
GitHub+1

2. Check Wi-Fi connection
After entering credentials, ESP32 should restart and attempt to connect.

Look for messages indicating:
- Wi-Fi connection successful.
- Device IP address.
- HomeSpan READY / pairing-needed status.
You can also type i in Serial Monitor to display startup diagnostics.
3. Pair with Apple Home
Once Wi-Fi is connected:
Default HomeSpan pairing code
`466-37-726`
On your iPhone:
1. Open Apple Home.
2. Tap + → Add Accessory.
3. Select More Options.
4. Find ESP32 Buzzer.
5. Enter `466-37-726`.
6. Accept the uncertified accessory warning.
HomeSpan's default pairing code is documented in its official guide. 
GitHub+1

4. Check these network requirements
- iPhone and ESP32 must be on the same local network.
- Use a 2.4 GHz Wi-Fi network for ESP32-C3.
- Disable guest-network client isolation.
- Ensure your router allows mDNS/Bonjour discovery.
5. If it still doesn't appear
Open Serial Monitor and enter:
?


This displays available HomeSpan commands.

If the device was previously paired and then removed while the ESP32 was offline, enter:
H


This clears the HomeKit pairing data and generates a fresh HomeKit device identity. It does not erase Wi-Fi credentials.

## How It Works

The project uses HomeSpan to expose a standard HomeKit Switch accessory.

The HomeKit `On` characteristic represents the buzzer state.

### System Architecture

```text
                 Apple Home App
                       |
                       |
                 HomeKit / Wi-Fi
                       |
                       v
              +----------------+
              |     HomeSpan   |
              |                |
              |  Switch Service|
              |  On Character. |
              +----------------+
                       |
                       v
              +----------------+
              | ESP32-C3 Super |
              |      Mini      |
              +----------------+
                  |        |
                  |        |
             GPIO 7     GPIO 6
                  |        |
                  v        v
               Passive   Physical
               Buzzer    Push Button
```

### Buzzer Control

The passive buzzer is controlled using the ESP32 LEDC hardware PWM peripheral.

| Parameter | Value |
|---|---|
| GPIO | 7 |
| Frequency | 2000 Hz |
| Resolution | 8 bits |
| Duty Cycle | 50% |
| PWM Duty Value | 128 |

When the buzzer is enabled, the ESP32 generates a 2 kHz PWM signal.

When disabled, the PWM duty is set to zero.

### Physical Button

The push button is configured using:

```cpp
pinMode(BUTTON_PIN, INPUT_PULLUP);
```

The button is active LOW.

A debounce interval of 50 milliseconds is used to prevent multiple toggles from a single press.

When pressed:

1. The current HomeKit switch state is read.
2. The state is inverted.
3. The HomeKit characteristic is updated.
4. The buzzer output is updated.
5. Apple Home receives the updated accessory state.

## Wi-Fi Configuration

HomeSpan provides an interactive Wi-Fi provisioning interface through the Serial Monitor.

1. Connect the ESP32 to your computer.
2. Open Arduino IDE Serial Monitor.
3. Set baud rate to `115200`.
4. Set the line ending to `Newline`.
5. Enter the HomeSpan Wi-Fi configuration command:

```text
W
```

6. Select your Wi-Fi network.
7. Enter your Wi-Fi password.

The credentials are stored in ESP32 flash memory.

The ESP32-C3 requires a compatible 2.4 GHz Wi-Fi network.

## Apple HomeKit Pairing

After successful Wi-Fi configuration:

1. Open the Serial Monitor.
2. Wait for HomeSpan to initialize.
3. Locate the HomeKit pairing code displayed by HomeSpan.
4. Open the Apple Home app.
5. Select **Add Accessory**.
6. Select **More Options** if the accessory is not immediately discovered.
7. Select the ESP32 Buzzer accessory.
8. Enter the displayed HomeKit pairing code.
9. Complete the pairing process.

The accessory will appear as a switch in Apple Home.

## Operating the Buzzer

### Using Apple Home

| Apple Home Action | ESP32 Behavior |
|---|---|
| Switch ON | Generates a 2 kHz tone |
| Switch OFF | Stops the buzzer |

### Using the Physical Button

| Button Action | ESP32 Behavior |
|---|---|
| Press once | Toggles buzzer ON/OFF |
| Press again | Toggles buzzer OFF/ON |

The physical button and Apple Home share the same HomeKit state.

## Power Cycle and Persistence

The system is designed to recover automatically after losing power.

When the ESP32 is powered OFF and ON again:

1. The ESP32 executes its startup routine.
2. HomeSpan loads previously saved Wi-Fi credentials.
3. The ESP32 reconnects to the configured Wi-Fi network.
4. HomeSpan restores its existing HomeKit accessory identity and pairing information.
5. The accessory reconnects to Apple Home.
6. The buzzer initializes in the OFF state.
7. The physical button becomes operational.

### Persistent and Non-Persistent Information

| Information | Persistence |
|---|---|
| Wi-Fi credentials | Persistent |
| HomeKit pairing information | Persistent |
| Accessory identity | Persistent |
| Buzzer state | Resets to OFF |
| GPIO configuration | Reinitialized on startup |

**Important:** A normal power cycle does not require HomeKit pairing again. However, erasing flash, changing partition layouts, or resetting HomeSpan pairing data can require reconfiguration or pairing.

## Troubleshooting

### Accessory Not Appearing in Apple Home

Check:

- ESP32 is connected to Wi-Fi.
- iPhone and ESP32 are on the same local network.
- Both devices use a compatible network.
- Router client isolation is disabled.
- mDNS/Bonjour traffic is permitted.

### Buzzer Does Not Produce Sound

Check:

- Buzzer is passive and correctly connected.
- Positive terminal is connected to GPIO 7.
- Negative terminal is connected to GND.
- PWM initialization succeeds.
- Buzzer operates at the selected frequency.

### Physical Button Does Not Work

Check:

- Button is connected between GPIO 6 and GND.
- `INPUT_PULLUP` is enabled.
- GPIO 6 is not being used by another peripheral.

### HomeKit Pairing Issues

Check HomeSpan's Serial Monitor output.

If necessary, use the HomeSpan pairing reset command documented in its user guide. This clears pairing information and requires pairing again.

## Project Dependencies

- [HomeSpan – Apple HomeKit Library for ESP32](https://github.com/HomeSpan/HomeSpan)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [HomeSpan User Guide](https://github.com/HomeSpan/HomeSpan/blob/master/docs/UserGuide.md)

## Future Improvements

- Adjustable buzzer frequency.
- Multiple buzzer tones.
- HomeKit automation support.
- Multiple independently controlled accessories.
- Custom HomeKit accessory identification.
- Battery-powered operation.
- OTA firmware updates.

## License

This project is intended for educational and personal DIY home automation use. Refer to the respective dependency licenses for HomeSpan and ESP32 Arduino Core.