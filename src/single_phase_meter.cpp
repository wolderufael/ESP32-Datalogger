#include "single_phase_meter.h"

ModbusMaster modbusNode;

// Callback to switch MAX485 to Transmit mode
void preTransmission() {
  digitalWrite(RE_DE, HIGH);
  delayMicroseconds(100);
}

// Callback to switch MAX485 to Receive mode
void postTransmission() {
  delayMicroseconds(100);
  digitalWrite(RE_DE, LOW);
}

void single_phase_meter_init() {
  Serial.println("Initializing Single Phase Meter (Modbus RS485)...");
  
  // Initialize Serial2 for Modbus communication
  Serial2.begin(MODBUS_BAUD_RATE, SERIAL_8N1, RXD2, TXD2);
  
  // Configure RE/DE pin for RS485 direction control
  pinMode(RE_DE, OUTPUT);
  digitalWrite(RE_DE, LOW);  // Start in receive mode
  
  // Initialize ModbusMaster with slave ID
  modbusNode.begin(MODBUS_SLAVE_ID, Serial2);
  
  // Register callbacks for RS485 direction control
  modbusNode.preTransmission(preTransmission);
  modbusNode.postTransmission(postTransmission);
  
  Serial.println("Single Phase Meter initialized successfully");
}

float read_voltage_from_meter() {
  uint8_t result = modbusNode.readHoldingRegisters(VOLTAGE_REGISTER, 1);
  
  if (result == modbusNode.ku8MBSuccess) {
    uint16_t rawValue = modbusNode.getResponseBuffer(0);
    // Convert raw value to voltage (adjust scaling factor as needed)
    // Assuming the register returns voltage in 0.1V units (e.g., 2300 = 230.0V)
    float voltage = rawValue / 10.0f;
    return voltage;
  } else {
    Serial.print("Modbus Error: 0x");
    Serial.println(result, HEX);
    return -1.0f;  // Return error value
  }
}

bool is_meter_connected() {
  //uint8_t result = modbusNode.readInputRegisters(VOLTAGE_REGISTER, 1);
  uint8_t result = modbusNode.readHoldingRegisters(VOLTAGE_REGISTER, 1);
  return (result == modbusNode.ku8MBSuccess);
}

