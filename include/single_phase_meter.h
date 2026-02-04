#ifndef SINGLE_PHASE_METER_H
#define SINGLE_PHASE_METER_H

#include <Arduino.h>
#include <ModbusMaster.h>

// Pin Definitions for RS485
#define RXD2 16
#define TXD2 17
#define RE_DE 4

// Modbus Configuration
#define MODBUS_SLAVE_ID 1
#define MODBUS_BAUD_RATE 9600
//#define VOLTAGE_REGISTER 0x000B // Register address for voltage
#define VOLTAGE_REGISTER 0x0100  // Register address for voltage

// Function declarations
void single_phase_meter_init();
float read_voltage_from_meter();
bool is_meter_connected();

extern ModbusMaster modbusNode;

#endif

