# DS18B20_OneWire
One Wire Temperature Sensor Breakout Board on Arduino Shield Format and STM32 Nucleo-F446 Based Software

# Features
- four channels 1-Wire Interface with onboard Driver-Stage 
- designed for DS18B20 Temperature Sensor but any 1-Wire Sensor / Actor would do (minor Software adjustments required)
- Industrial ready CAN-Bus interface
    - works with standard (11-Bit) and extended (29-Bit) CAN identifier
- Industrial ready Modbus interface will come soon
- Arduino-Shield layout will fit on several Dev-Boards including all Nucleo-64 boards and Arduino-UNO boards

1Wire Search Algorithm and Driver Stage are basically taken from MAXIM Datasheet and Microship App-Notes.

![1Wire Arduino Shield](/Images/DS18b20_Arduino_Shield.png)

Quattro Formaggi Quattro Stagioni

to study some of my testing done sofar please have a look [here](/Testing/README.md)

# ToDo
- add User-LED on Arduino Shield (Nucleo LED is covered) [HW]
- add Temperature Results to CAN Message [SW]
- use bigger terminal blocks for 1-Wire Interface [HW]
- use bigger terminal blocks for Power Supply and CAN [HW]