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

// Register addresses (Holding Registers)
#define SINGLE_PHASE_REG_VOLTAGE 0x000B
#define SINGLE_PHASE_REG_CURRENT 0x000E
#define SINGLE_PHASE_REG_FREQUENCY 0x0011

// Data structure to hold all single phase meter readings
struct SinglePhaseMeterData {
  float voltage;
  float current;
  float frequency;
  bool is_valid;
};

// Function declarations
void single_phase_meter_init();
float read_voltage_register();    // Read voltage register (multiplier: 0.1)
float read_current_register();   // Read current register (multiplier: 0.1)
float read_frequency_register(); // Read frequency register (multiplier: 0.01)
float read_voltage_from_meter();  // Legacy function for backward compatibility
bool read_single_phase_meter_data(SinglePhaseMeterData* data);
bool is_meter_connected();

extern ModbusMaster modbusNode;

#endif

