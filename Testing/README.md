# Testing
Because of lacking a proper CAN-Testing-Tool like this [PCAN-USB](https://www.peak-system.com/PCAN-USB.199.0.html?&L=1) I just used a Arduino with CAN-Shield and a minimal Arduino Sketch.
The Arduino program sends periodic test messages and receives CAN-messages from the Nucleo Board. With this I could ensure CAN (especially the CAN transceiver IC) works both directions.

![arrangement Arduino and Nucleo](../Images/quattro_formaggi_1.png)

![arrangement Arduino and Nucleo 2](../Images/quattro_formaggi_2.png)

![terminal startup message](../Images/terminal_startup.png)

![terminal search and read DS18B20 sensors](../Images/terminal_read_temperatures.png)

![terminal send and receive CAN messages](../Images/terminal_read_temperatures_rx_can.png)

![Arduino terminal send and receive CAN messages](../Images/testing_RX_TX_CAN.png)

## Hardware
todo

## Software
todo

## CAN-Bus
todo