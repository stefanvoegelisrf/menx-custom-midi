# Project Setup - Software

Below is a description of what is required to get the software of this project to work.

- [Project Setup - Software](#project-setup---software)
  - [Prerequisites](#prerequisites)
    - [Install Arduino IDE](#install-arduino-ide)
    - [Install Adafruit Seesaw library](#install-adafruit-seesaw-library)
    - [Install MIDI Library](#install-midi-library)
    - [Install ESP32 Boards](#install-esp32-boards)
    - [MIDI definition](#midi-definition)
      - [Data Value Mapping](#data-value-mapping)
  - [Troubleshooting](#troubleshooting)
    - [No serial messages received](#no-serial-messages-received)
    - [No seesaw chip found](#no-seesaw-chip-found)
  - [Resources](#resources)


## Prerequisites

In order to get started with the development, we need to set up a few things first.

### Install Arduino IDE

To develop this project, we use the Arduino IDE, as it is simple to use and can help us bundling the code correctly for our ESP32.

To install it, download it from [arduino.cc](https://www.arduino.cc/en/software/) and follow the installation process.

### Install Adafruit Seesaw library

The Adafruit components used in this project all have seesaw chips, so we need to install the seesaw library inside of the Arduino IDE to communicate with these chips.

To install it, go to the library manager and search for `adafruit seesaw` and install the library.

![Screenshot for Adafruit Seesaw library installation inside of Arduino IDE](./images/project-setup/adafruit-seesaw-installation.png)

### Install MIDI Library

This project transmits the values of the components via MIDI over USB.

TODO: install MIDI library and define MIDI mapping

### Install ESP32 Boards

As we use an ESP32 board in this project, we need to add the board configuration.

To do so, open the board manager and search for `esp32` and install the board configurations.

![Screenshot for ESP32 board configuration installation inside of Arduino IDE](./images/project-setup/esp32-board-configuration-installation.png)

### MIDI definition

My layout enables me to do a MIDI mapping to two channels. Like this I can assign 3 rotary encoders, 1 slider and 4 keys to each channel.

| MIDI Channel 1   | MIDI Channel 2    |
| ---------------- | ----------------- |
| Left Slider      | Right Slider      |
| Left Eye Rotary  | Right Eye Rotary  |
| Left Nose Rotary | Right Nose Rotary |
| Left Ear Rotary  | Right Ear Rotary  |
| Top 4 Keys       | Bottom 4 Keys     |

#### Data Value Mapping

The values that I receive from my components need to be mapped to MIDI values. The table below lines out how this is done:

| Hardware Action    | MIDI Message Type   | Data 1 (Identification) | Data 2 (Value) |
| ------------------ | ------------------- | ----------------------- | -------------- |
| Slider Movement    | Control Change (CC) | CC Number               | 0 to 127       |
| Encoder Turn Right | Control Change (CC) | CC Number               | 1              |
| Encoder Turn Left  | Control Change (CC) | CC Number               | 127            |
| Encoder/Key Press  | Note On or CC       | Note/CC Number          | 127            |
| Encoder/Key Lift   | Note Off or CC      | Note/CC Number          | 0              |

## Troubleshooting

I have run into some issues in this project and here is a list of issues that I ran into, so if I run into them again, I know what the issue is and how to solve it.

### No serial messages received

I had the issue that I did not receive serial messages from my board, which sucks when trying to debug the project.

The issue was, that I had the setting `USB CDC On Boot` on `Disabled`. To receive the serial messages, set this to `Enabled`.

![Screenshot of setting USB CDC On Boot to Enabled inside of Arduino IDE](./images/project-setup/issue-usb-cdc-on-boot-disabled.png)

### No seesaw chip found

When using an ESP32, we can define the I2C pins freely and most likely, they will not be the standard I2C pins.

## Resources

- [learn.adafruit.com - Adafruit Seesaw](https://learn.adafruit.com/adafruit-seesaw-atsamd09-breakout/overview)
- [learn.adafruit.com - Adafruit NeoSlider Arduino](https://learn.adafruit.com/adafruit-neoslider/arduino)
- [docs.waveshare.com - ESP32-S3-DEV-KIT-N8R8 Development Environment Setup Arduino](https://docs.waveshare.com/ESP32-S3-DEV-KIT-N8R8/Development-Environment-Setup-Arduino)
- [docs.waveshare.com - ESP32 Arduino IDE Setup](https://docs.waveshare.com/ESP32-Arduino-Tutorials/Arduino-IDE-Setup)
- [learn.adafruit.com - Arduino MIDI Example](https://learn.adafruit.com/adafruit-midi-featherwing/arduino-midi-example)
- [learn.adafruit.com - MIDI for Makers](https://learn.adafruit.com/midi-for-makers/overview)