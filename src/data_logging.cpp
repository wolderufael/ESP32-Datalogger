#include <FS.h>
#include <SPIFFS.h>
#include "vibrating_wire.h"
#include "data_logging.h"
#include "configuration.h"
#include "utils.h"
#include "single_phase_meter.h"
#include "srne_inverter.h"
#include "mqtt.h"

// Sensor Libs
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

unsigned long lastLogTime[CHANNEL_COUNT] = {0};

float generateRandomFloat(float minVal, float maxVal) {
  uint32_t randomValue = esp_random();
  float scaledValue = (float)randomValue / (float)UINT32_MAX; // Scale to [0, 1]
  return minVal + scaledValue * (maxVal - minVal); // Scale to [minVal, maxVal]
}

void logDataFunction(int channel, String timestamp) {
  float sensorValue = 0.0f;
  const char* sensorTypeStr = "Unknown";
  
  // Read sensor data based on sensor type
  switch (dataConfig.type[channel]) {
    case SinglePhaseMeter:
      sensorTypeStr = "SinglePhaseMeter";
      // Publish all meter data as JSON
      if (!publish_single_phase_meter_data(channel, timestamp.c_str())) {
        Serial.printf("Channel %d: Failed to read/publish single phase meter data\n", channel);
        return;  // Skip if read/publish failed
      }
      break;
      
    case SRNEInverter:
      sensorTypeStr = "SRNEInverter";
      // Publish all inverter data as JSON
      if (!publish_srne_inverter_data(channel, timestamp.c_str())) {
        Serial.printf("Channel %d: Failed to read/publish SRNE inverter data\n", channel);
        return;  // Skip if read/publish failed
      }
      break;
      
    case VibratingWire:
      // TODO: Implement vibrating wire reading
      sensorValue = generateRandomFloat(7000, 8000);
      sensorTypeStr = "VibratingWire";
      // Publish directly to MQTT (no SD card)
      publish_sensor_data(channel, sensorTypeStr, sensorValue, timestamp.c_str());
      break;
      
    case Barometric:
      // TODO: Implement barometric sensor reading
      sensorValue = generateRandomFloat(950, 1050);
      sensorTypeStr = "Barometric";
      // Publish directly to MQTT (no SD card)
      publish_sensor_data(channel, sensorTypeStr, sensorValue, timestamp.c_str());
      break;
      
    default:
      Serial.printf("Channel %d: Unknown sensor type\n", channel);
      return;
  }
  
  // Update latest data in dataconfig
  time_t now;
  time(&now);  // Get the current time as time_t (epoch time)
  dataConfig.time[channel] = now;
}

void logDataTask(void *parameter) {
  while (true) {
    unsigned long currentTime = millis() / 1000; // Convert milliseconds to seconds

    for (int i = 0; i < CHANNEL_COUNT; i++) {
      if (dataConfig.enabled[i] && (currentTime - lastLogTime[i] >= dataConfig.interval[i])) {
        logDataFunction(i, get_current_time(false));
        lastLogTime[i] = currentTime;
      }
      vTaskDelay(100 / portTICK_PERIOD_MS); // Delay for 100 milliseconds
    }
  }
}

void log_data_init() {

  Serial.println("Initializing data logging (MQTT mode - no SD card).");
  
  // Initialize single phase meter if any channel uses it
  bool hasSinglePhaseMeter = false;
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    if (dataConfig.enabled[i] && dataConfig.type[i] == SinglePhaseMeter) {
      hasSinglePhaseMeter = true;
      break;
    }
  }
  
  if (hasSinglePhaseMeter) {
    single_phase_meter_init();
  }
  
  // Initialize SRNE inverter if any channel uses it
  bool hasSRNEInverter = false;
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    if (dataConfig.enabled[i] && dataConfig.type[i] == SRNEInverter) {
      hasSRNEInverter = true;
      break;
    }
  }
  
  if (hasSRNEInverter) {
    srne_inverter_init();
  }

  // Print enabled channels
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    if (dataConfig.enabled[i]) {
      Serial.printf("Channel %d: Enabled, Type: %d, Interval: %d minutes\n", 
                    i, dataConfig.type[i], dataConfig.interval[i]);
    }
  }

  xTaskCreate(
    logDataTask,        // Task function
    "Log Data Task",    // Name of the task (for debugging)
    10000,              // Stack size (in words, not bytes)
    NULL,               // Task input parameter
    1,                  // Priority of the task
    NULL                // Task handle
  );
  Serial.println("Added Data Logging Task (MQTT direct mode).");

}