#include "srne_inverter.h"

ModbusMaster srneModbusNode;

// Callback to switch MAX485 to Transmit mode
void srne_preTransmission() {
  digitalWrite(RE_DE, HIGH);
  delayMicroseconds(100);
}

// Callback to switch MAX485 to Receive mode
void srne_postTransmission() {
  delayMicroseconds(100);
  digitalWrite(RE_DE, LOW);
}

void srne_inverter_init() {
  Serial.println("Initializing SRNE Inverter (Modbus RS485)...");
  
  // Initialize Serial2 for Modbus communication (shared with single-phase meter)
  Serial2.begin(SRNE_MODBUS_BAUD_RATE, SERIAL_8N1, RXD2, TXD2);
  
  // Configure RE/DE pin for RS485 direction control
  pinMode(RE_DE, OUTPUT);
  digitalWrite(RE_DE, LOW);  // Start in receive mode
  
  // Small delay to allow hardware to stabilize
  delay(100);
  
  // Initialize ModbusMaster with slave ID
  srneModbusNode.begin(SRNE_MODBUS_SLAVE_ID, Serial2);
  
  // Try to set longer timeout (if supported by library)
  // Some ModbusMaster versions support setResponseTimeout()
  // srneModbusNode.setResponseTimeout(2000); // 2 second timeout
  
  // Register callbacks for RS485 direction control
  srneModbusNode.preTransmission(srne_preTransmission);
  srneModbusNode.postTransmission(srne_postTransmission);
  
  Serial.println("SRNE Inverter initialized successfully");
}

float read_srne_register(uint16_t registerAddress) {
  uint8_t result = srneModbusNode.readHoldingRegisters(registerAddress, 1);
  
  // Always yield to other tasks after Modbus operation to prevent watchdog timeout
  vTaskDelay(5 / portTICK_PERIOD_MS);  // 5ms delay - allows other tasks to run
  
  if (result == srneModbusNode.ku8MBSuccess) {
    uint16_t rawValue = srneModbusNode.getResponseBuffer(0);
    // For now, use multiplier 1.0 (will be updated later)
    float value = rawValue * 1.0f;
    return value;
  } else {
    // Only print error for first few failures to avoid spam
    static int errorCount = 0;
    if (errorCount < 5) {
      Serial.print("Modbus Error reading register 0x");
      Serial.print(registerAddress, HEX);
      Serial.print(": 0x");
      Serial.println(result, HEX);
      errorCount++;
    }
    return -9999.0f;  // Return error value
  }
}

bool read_srne_inverter_data(SRNEInverterData* data) {
  if (data == nullptr) {
    return false;
  }
  
  // Initialize structure
  memset(data, 0, sizeof(SRNEInverterData));
  data->is_valid = false;
  
  unsigned long startTime = millis();
  
  // Read all registers with error handling
  // Critical battery parameters first
  data->battery_soc = read_srne_register(SRNE_REG_BATTERY_SOC);
  data->battery_voltage = read_srne_register(SRNE_REG_BATTERY_VOLTAGE);
  data->battery_current = read_srne_register(SRNE_REG_BATTERY_CURRENT);
  
  // PV parameters
  data->pv_voltage = read_srne_register(SRNE_REG_PV_VOLTAGE);
  data->pv_current = read_srne_register(SRNE_REG_PV_CURRENT);
  data->pv_power = read_srne_register(SRNE_REG_PV_POWER);
  data->battery_charge_power = read_srne_register(SRNE_REG_BATTERY_CHARGE_POWER);
  
  // Configuration registers (may fail, continue anyway)
  data->battery_type = read_srne_register(SRNE_REG_BATTERY_TYPE);
  data->battery_over_voltage = read_srne_register(SRNE_REG_BATTERY_OVER_VOLTAGE);
  data->battery_equalizing_charge_voltage = read_srne_register(SRNE_REG_BATTERY_EQUALIZING_CHARGE_VOLTAGE);
  data->battery_boost_charge_voltage = read_srne_register(SRNE_REG_BATTERY_BOOST_CHARGE_VOLTAGE);
  data->battery_float_charge_voltage = read_srne_register(SRNE_REG_BATTERY_FLOAT_CHARGE_VOLTAGE);
  data->over_discharge_delay_time = read_srne_register(SRNE_REG_OVER_DISCHARGE_DELAY_TIME);
  data->battery_equalizing_charge_time = read_srne_register(SRNE_REG_BATTERY_EQUALIZING_CHARGE_TIME);
  data->battery_equalizing_interval = read_srne_register(SRNE_REG_BATTERY_EQUALIZING_INTERVAL);
  data->battery_under_voltage_warning = read_srne_register(SRNE_REG_BATTERY_UNDER_VOLTAGE_WARNING);
  data->battery_over_discharge_voltage = read_srne_register(SRNE_REG_BATTERY_OVER_DISCHARGE_VOLTAGE);
  data->battery_limited_discharge_voltage = read_srne_register(SRNE_REG_BATTERY_LIMITED_DISCHARGE_VOLTAGE);
  data->battery_boost_charge_time = read_srne_register(SRNE_REG_BATTERY_BOOST_CHARGE_TIME);
  data->battery_mains_switching_voltage = read_srne_register(SRNE_REG_BATTERY_MAINS_SWITCHING_VOLTAGE);
  data->battery_stop_charging_current = read_srne_register(SRNE_REG_BATTERY_STOP_CHARGING_CURRENT);
  data->battery_number_in_series = read_srne_register(SRNE_REG_BATTERY_NUMBER_IN_SERIES);
  data->inverter_switch_voltage = read_srne_register(SRNE_REG_INVERTER_SWITCH_VOLTAGE);
  data->battery_max_charge_current = read_srne_register(SRNE_REG_BATTERY_MAX_CHARGE_CURRENT);
  data->inverter_output_priority = read_srne_register(SRNE_REG_INVERTER_OUTPUT_PRIORITY);
  data->inverter_charge_priority = read_srne_register(SRNE_REG_INVERTER_CHARGE_PRIORITY);
  data->grid_battery_charge_max_current = read_srne_register(SRNE_REG_GRID_BATTERY_CHARGE_MAX_CURRENT);
  data->inverter_charger_priority = read_srne_register(SRNE_REG_INVERTER_CHARGER_PRIORITY);
  data->inverter_alarm_control = read_srne_register(SRNE_REG_INVERTER_ALARM_CONTROL);
  
  // Status registers
  data->machine_state = read_srne_register(SRNE_REG_MACHINE_STATE);
  data->total_running_days = read_srne_register(SRNE_REG_TOTAL_RUNNING_DAYS);
  
  // Grid and inverter parameters
  data->grid_voltage = read_srne_register(SRNE_REG_GRID_VOLTAGE);
  data->grid_input_current = read_srne_register(SRNE_REG_GRID_INPUT_CURRENT);
  data->grid_frequency = read_srne_register(SRNE_REG_GRID_FREQUENCY);
  data->inverter_voltage = read_srne_register(SRNE_REG_INVERTER_VOLTAGE);
  data->inverter_current = read_srne_register(SRNE_REG_INVERTER_CURRENT);
  data->inverter_frequency = read_srne_register(SRNE_REG_INVERTER_FREQUENCY);
  data->load_current = read_srne_register(SRNE_REG_LOAD_CURRENT);
  data->inverter_power = read_srne_register(SRNE_REG_INVERTER_POWER);
  data->inverter_apparent_power = read_srne_register(SRNE_REG_INVERTER_APPARENT_POWER);
  data->grid_battery_charge_current = read_srne_register(SRNE_REG_GRID_BATTERY_CHARGE_CURRENT);
  
  // Temperature sensors
  data->temp_dc = read_srne_register(SRNE_REG_TEMP_DC);
  data->temp_ac = read_srne_register(SRNE_REG_TEMP_AC);
  data->temp_tr = read_srne_register(SRNE_REG_TEMP_TR);
  data->pv_battery_charge_current = read_srne_register(SRNE_REG_PV_BATTERY_CHARGE_CURRENT);
  
  unsigned long elapsedTime = millis() - startTime;
  Serial.printf("SRNE: Read 46 registers in %lu ms\n", elapsedTime);
  
  // Check if at least some critical registers were read successfully
  // (not all -9999.0f values)
  if (data->battery_soc != -9999.0f || data->battery_voltage != -9999.0f || 
      data->pv_voltage != -9999.0f || data->grid_voltage != -9999.0f) {
    data->is_valid = true;
    return true;
  }
  
  return false;
}

bool is_srne_inverter_connected() {
  uint8_t result = srneModbusNode.readHoldingRegisters(SRNE_REG_MACHINE_STATE, 1);
  return (result == srneModbusNode.ku8MBSuccess);
}

