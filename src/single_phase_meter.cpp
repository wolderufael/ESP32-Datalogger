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

// Read a single holding register
static float read_single_phase_register(uint16_t registerAddress) {
  uint8_t result = modbusNode.readHoldingRegisters(registerAddress, 1);
  
  if (result == modbusNode.ku8MBSuccess) {
    uint16_t rawValue = modbusNode.getResponseBuffer(0);
    // Convert raw value to float (assuming 0.1 units scaling, e.g., 2300 = 230.0)
    float value = rawValue / 10.0f;
    return value;
  } else {
    // Limit error spam - only print occasionally
    static unsigned long lastErrorTime = 0;
    unsigned long now = millis();
    if (now - lastErrorTime > 5000) {  // Print error max once per 5 seconds
      Serial.printf("Single Phase Meter Modbus Error at 0x%04X: 0x%02X\n", registerAddress, result);
      lastErrorTime = now;
    }
    return -1.0f;  // Return error value
  }
}

// Legacy function for backward compatibility
float read_voltage_from_meter() {
  return read_single_phase_register(SINGLE_PHASE_REG_VOLTAGE);
}

// Read all single phase meter data
bool read_single_phase_meter_data(SinglePhaseMeterData* data) {
  if (data == nullptr) {
    return false;
  }
  
  // Initialize data structure
  data->voltage = -1.0f;
  data->current = -1.0f;
  data->frequency = -1.0f;
  data->is_valid = false;
  
  unsigned long startTime = millis();
  
  // Read voltage
  data->voltage = read_single_phase_register(SINGLE_PHASE_REG_VOLTAGE);
  vTaskDelay(5 / portTICK_PERIOD_MS);  // Yield CPU between reads
  
  // Read current
  data->current = read_single_phase_register(SINGLE_PHASE_REG_CURRENT);
  vTaskDelay(5 / portTICK_PERIOD_MS);  // Yield CPU between reads
  
  // Read frequency
  data->frequency = read_single_phase_register(SINGLE_PHASE_REG_FREQUENCY);
  
  // Check if at least one critical reading succeeded
  data->is_valid = (data->voltage >= 0.0f || data->current >= 0.0f || data->frequency >= 0.0f);
  
  unsigned long elapsedTime = millis() - startTime;
  Serial.printf("Single Phase Meter: Read 3 registers in %lu ms\n", elapsedTime);
  
  if (data->is_valid) {
    Serial.printf("Single Phase Meter - Voltage: %.2f V, Current: %.2f A, Frequency: %.2f Hz\n",
                  data->voltage, data->current, data->frequency);
  }
  
  return data->is_valid;
}

bool is_meter_connected() {
  uint8_t result = modbusNode.readHoldingRegisters(SINGLE_PHASE_REG_VOLTAGE, 1);
  return (result == modbusNode.ku8MBSuccess);
}

