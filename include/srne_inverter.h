#ifndef SRNE_INVERTER_H
#define SRNE_INVERTER_H

#include <Arduino.h>
#include <ModbusMaster.h>

// Pin Definitions for RS485 (shared with single-phase meter)
#define RXD2 16
#define TXD2 17
#define RE_DE 4

// Modbus Configuration
#define SRNE_MODBUS_SLAVE_ID 1
#define SRNE_MODBUS_BAUD_RATE 9600

// Register Addresses (Holding Registers)
#define SRNE_REG_BATTERY_SOC 0x0100
#define SRNE_REG_BATTERY_VOLTAGE 0x0101
#define SRNE_REG_BATTERY_CURRENT 0x0102
#define SRNE_REG_PV_VOLTAGE 0x0107
#define SRNE_REG_PV_CURRENT 0x0108
#define SRNE_REG_PV_POWER 0x0109
#define SRNE_REG_BATTERY_CHARGE_POWER 0x010E
#define SRNE_REG_BATTERY_TYPE 0xE004
#define SRNE_REG_BATTERY_OVER_VOLTAGE 0xE005
#define SRNE_REG_BATTERY_EQUALIZING_CHARGE_VOLTAGE 0xE007
#define SRNE_REG_BATTERY_BOOST_CHARGE_VOLTAGE 0xE008
#define SRNE_REG_BATTERY_FLOAT_CHARGE_VOLTAGE 0xE009
#define SRNE_REG_OVER_DISCHARGE_DELAY_TIME 0xE010
#define SRNE_REG_BATTERY_EQUALIZING_CHARGE_TIME 0xE011
#define SRNE_REG_BATTERY_EQUALIZING_INTERVAL 0xE013
#define SRNE_REG_BATTERY_UNDER_VOLTAGE_WARNING 0xE00C
#define SRNE_REG_BATTERY_OVER_DISCHARGE_VOLTAGE 0xE00D
#define SRNE_REG_BATTERY_LIMITED_DISCHARGE_VOLTAGE 0xE00E
#define SRNE_REG_BATTERY_BOOST_CHARGE_TIME 0xE012
#define SRNE_REG_BATTERY_MAINS_SWITCHING_VOLTAGE 0xE01B
#define SRNE_REG_BATTERY_STOP_CHARGING_CURRENT 0xE01C
#define SRNE_REG_BATTERY_NUMBER_IN_SERIES 0xE020
#define SRNE_REG_INVERTER_SWITCH_VOLTAGE 0xE022
#define SRNE_REG_BATTERY_MAX_CHARGE_CURRENT 0xE20A
#define SRNE_REG_INVERTER_OUTPUT_PRIORITY 0xE204
#define SRNE_REG_INVERTER_CHARGE_PRIORITY 0xE20F
#define SRNE_REG_GRID_BATTERY_CHARGE_MAX_CURRENT 0xE205
#define SRNE_REG_INVERTER_CHARGER_PRIORITY 0xE20F
#define SRNE_REG_INVERTER_ALARM_CONTROL 0xE210
#define SRNE_REG_MACHINE_STATE 0x0210
#define SRNE_REG_TOTAL_RUNNING_DAYS 0xF031
#define SRNE_REG_GRID_VOLTAGE 0x0213
#define SRNE_REG_GRID_INPUT_CURRENT 0x0214
#define SRNE_REG_GRID_FREQUENCY 0x0215
#define SRNE_REG_INVERTER_VOLTAGE 0x0216
#define SRNE_REG_INVERTER_CURRENT 0x0217
#define SRNE_REG_INVERTER_FREQUENCY 0x0218
#define SRNE_REG_LOAD_CURRENT 0x0219
#define SRNE_REG_INVERTER_POWER 0x021B
#define SRNE_REG_INVERTER_APPARENT_POWER 0x021C
#define SRNE_REG_GRID_BATTERY_CHARGE_CURRENT 0x021E
#define SRNE_REG_TEMP_DC 0x0221
#define SRNE_REG_TEMP_AC 0x0222
#define SRNE_REG_TEMP_TR 0x0223
#define SRNE_REG_PV_BATTERY_CHARGE_CURRENT 0x0224

// Structure to hold all inverter data
struct SRNEInverterData {
  float battery_soc;
  float battery_voltage;
  float battery_current;
  float pv_voltage;
  float pv_current;
  float pv_power;
  float battery_charge_power;
  float battery_type;
  float battery_over_voltage;
  float battery_equalizing_charge_voltage;
  float battery_boost_charge_voltage;
  float battery_float_charge_voltage;
  float over_discharge_delay_time;
  float battery_equalizing_charge_time;
  float battery_equalizing_interval;
  float battery_under_voltage_warning;
  float battery_over_discharge_voltage;
  float battery_limited_discharge_voltage;
  float battery_boost_charge_time;
  float battery_mains_switching_voltage;
  float battery_stop_charging_current;
  float battery_number_in_series;
  float inverter_switch_voltage;
  float battery_max_charge_current;
  float inverter_output_priority;
  float inverter_charge_priority;
  float grid_battery_charge_max_current;
  float inverter_charger_priority;
  float inverter_alarm_control;
  float machine_state;
  float total_running_days;
  float grid_voltage;
  float grid_input_current;
  float grid_frequency;
  float inverter_voltage;
  float inverter_current;
  float inverter_frequency;
  float load_current;
  float inverter_power;
  float inverter_apparent_power;
  float grid_battery_charge_current;
  float temp_dc;
  float temp_ac;
  float temp_tr;
  float pv_battery_charge_current;
  bool is_valid;  // Flag to indicate if data was successfully read
};

// Function declarations
void srne_inverter_init();
bool read_srne_inverter_data(SRNEInverterData* data);
float read_srne_register(uint16_t registerAddress);
bool is_srne_inverter_connected();

extern ModbusMaster srneModbusNode;

#endif

