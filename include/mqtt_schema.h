#ifndef MQTT_SCHEMA_H
#define MQTT_SCHEMA_H

#include <Arduino.h>

// Schema for register data points
// Each data point contains: name, value, unit, timestamp
struct RegisterDataPoint {
  const char* name;    // Human-readable name (e.g., "Voltage", "Battery SOC")
  float value;         // Numeric value
  const char* unit;    // Unit of measurement (e.g., "V", "A", "Hz", "%", "W")
  const char* timestamp; // ISO timestamp when this reading was taken
};

// Helper function to build JSON payload from data points
// Returns the JSON string
String build_sensor_json_payload(int channel, const char* sensorType, const char* timestamp, 
                                 RegisterDataPoint* dataPoints, int dataPointCount);

#endif

