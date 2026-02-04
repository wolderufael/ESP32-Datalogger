#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
// #include <SD.h>  // Disabled - no SD card needed
#include "configuration.h"
#include "srne_inverter.h"
#include "single_phase_meter.h"
// #include "LoRaLite.h"  // Disabled - no LoRa needed

// MQTT credentials - now loaded from systemConfig
// These are kept for backward compatibility but will be overridden
const char* mqtt_server_default = "192.168.8.1";
const char* mqtt_user_default = "senselynk";
const char* mqtt_password_default = "senselynk";

// Public Variables
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int mqtt_buffer_size = 4096;
SemaphoreHandle_t mqttMutex;

// Wrap MQTT operations with mutex
bool safe_mqtt_publish(const char* topic, const char* payload) {
    if (xSemaphoreTake(mqttMutex, portMAX_DELAY) == pdTRUE) {
        bool result = client.publish(topic, payload);
        xSemaphoreGive(mqttMutex);
        return result;
    }
    return false;
}

// ***********************************
// * MQTT Reconnect
// ***********************************
void mqtt_reconnect() {
  // Don't try to reconnect if already connected
  if (client.connected()) {
    return;
  }
  
  // Check if WiFi is connected first
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("MQTT: WiFi not connected, skipping MQTT reconnect");
    return;
  }
  
  Serial.print("Attempting MQTT connection...");
  
  // Get MQTT credentials from systemConfig
  const char* mqtt_server = (strlen(systemConfig.MQTT_SERVER) > 0) ? systemConfig.MQTT_SERVER : mqtt_server_default;
  const char* mqtt_user = (strlen(systemConfig.MQTT_USER) > 0) ? systemConfig.MQTT_USER : mqtt_user_default;
  const char* mqtt_password = (strlen(systemConfig.MQTT_PASSWORD) > 0) ? systemConfig.MQTT_PASSWORD : mqtt_password_default;
  
  Serial.printf(" to %s...", mqtt_server);
  
  // Update server if changed
  client.setServer(mqtt_server, 1883);
  
  // Attempt to connect with timeout
  if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
    Serial.println("connected");
    // Subscribe
    client.subscribe("esp32/output");
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again later");
    // Don't loop here - let the keepalive task handle retries
  }
}

// ***********************************
// MQTT Publish data from specified file
// NOTE: Disabled - SD card not used in this configuration
// ***********************************
/*
int mqtt_process_file(String filename, String DeviceName) {

    // mqtt client prepare
    if (!client.connected()) {
        mqtt_reconnect();
    }
    client.loop();

    // Prepare topic
    int lastSlashIndex = filename.lastIndexOf('/');
    String extractedFilename = filename.substring(lastSlashIndex + 1);
    String topic = DeviceName + "/" + extractedFilename;

    // load meta file
    size_t lastSentPosition = 0;
    String filenameStr = filename;
    int dotIndex = filenameStr.lastIndexOf('.');
    if (dotIndex > 0) {
        filenameStr = filenameStr.substring(0, dotIndex);
    }
    String metaFilename = filenameStr + ".meta";
    File metaFile = SD.open(metaFilename.c_str(), FILE_READ);
    if (!metaFile) {
      Serial.println("Meta file does not exist. Creating new meta file.");
      metaFile = SD.open(metaFilename.c_str(), FILE_WRITE);
      if (!metaFile) {
        Serial.println("Failed to create meta file!");
        return -1;
      }
      metaFile.println(0); // Write initial position 0 to the meta file
    } else {
      // Read the last sent position from the .meta file
      lastSentPosition = metaFile.parseInt();
    }
    metaFile.close();
    Serial.println("Meta file loaded.");

    // Open the file for reading
    File dataFile = SD.open(filename);
    if (!dataFile) {
        Serial.println("Failed to open file for reading");
        return -2;
    }
    Serial.println("Processing data file.");


    // Check if the lastSentPosition is larger than the file size
    size_t fileSize = dataFile.size();
    if (lastSentPosition > fileSize) {
        Serial.println("Last sent position is beyond the end of the file. Resetting meta file.");
        dataFile.close();

        // Reset the meta file to 0
        metaFile = SD.open(metaFilename.c_str(), FILE_WRITE);
        if (!metaFile) {
            Serial.println("Failed to reset meta file!");
            dataFile.close();
            return -3;
        }
        metaFile.println(0);
        metaFile.close();
        return -4;
    }

    if (dataFile.seek(lastSentPosition)) {
        Serial.println("Seek successful.");
        // Continue processing the file from this position
    } else {
        Serial.println("Seek failed.");
        // Handle the error, possibly by resetting the position or aborting the operation
    }


    // Initialize performance measurement
    unsigned long startTime = millis();
    size_t totalBytesSent = 0;

    Serial.println("Enter publish loop");
    // Read the file line by line
    while (dataFile.available()) {
        String line = dataFile.readStringUntil('\n');  // Read until the newline character

        // Skip the first line (header)
        if (line.startsWith("Time,")) {
            continue;
        }

        // Publish the line to the MQTT topic
        if (safe_mqtt_publish(topic.c_str(), line.c_str())) {
            // Add the size of the line to the total bytes sent
            totalBytesSent += line.length();
        }
    }

    // Close the file
    lastSentPosition = dataFile.position(); // Update the current position
    dataFile.close();
    Serial.println("Closed data file.");

    // Update the meta file with the last sent position
    metaFile = SD.open(metaFilename.c_str(), FILE_WRITE);
    if (!metaFile) {
        Serial.println("Failed to open meta file for updating!");
        return -5;
    }
    metaFile.seek(0);
    metaFile.println(lastSentPosition); // Write the current position to the meta file
    metaFile.close();
    Serial.println("Updated meta file.");

    // Calculate time taken and transfer rate
    unsigned long endTime = millis();
    unsigned long duration = endTime - startTime;  // Duration in milliseconds
    float transferRateKBps = (totalBytesSent / 1024.0) / (duration / 1000.0);  // KB/s

    // Output performance metrics
    Serial.print("File processing complete. Time taken: ");
    Serial.print(duration / 1000.0);  // Convert to seconds
    Serial.print(" seconds, Transfer rate: ");
    Serial.print(transferRateKBps);
    Serial.println(" KB/s");
    return 0;
}
*/

// **************************************
// * Send All .dat File From Folder
// NOTE: Disabled - SD card not used in this configuration
// **************************************
/*
bool mqtt_process_folder(String folderPath, String extension, String DeviceName){
  File root = SD.open(folderPath);

  if (!root) {
    Serial.println("Failed to open directory: " + folderPath);
    return -1;
  }

  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    if (!file.isDirectory() && fileName.endsWith(extension)) {
      String fullFilePath = folderPath + "/" + fileName;
      Serial.print("== Process file: "); Serial.println(fullFilePath);

      int res = mqtt_process_file(fullFilePath, DeviceName);
      if ( res != 0) {// append mode

        // bad meta file, resend entire file
        if (res == -4) {
          continue;
        }
        Serial.println("sync_folder: send file failed, abort.");
        file.close();
        return -2;
      }
      file.close();
      Serial.print("Closed file:"); Serial.println(fullFilePath);
    }
    file = root.openNextFile();
  }
  root.close();
  return 0;
}
*/

void publish_system_status() {
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();

  // Gather system status
  int cpuFreq = getCpuFrequencyMhz();
  uint32_t freeHeap = esp_get_free_heap_size();
  uint32_t minFreeHeap = esp_get_minimum_free_heap_size();

  // Calculate uptime
  unsigned long uptimeMillis = millis();
  unsigned long uptimeSeconds = uptimeMillis / 1000;
  unsigned long uptimeMinutes = uptimeSeconds / 60;
  unsigned long uptimeHours = uptimeMinutes / 60;
  unsigned long uptimeDays = uptimeHours / 24;

  uptimeHours = uptimeHours % 24;
  uptimeMinutes = uptimeMinutes % 60;
  uptimeSeconds = uptimeSeconds % 60;

  // Create the message payload
  String payload = "CPU Frequency: " + String(cpuFreq) + " MHz\n";
  payload += "Free Heap: " + String(freeHeap) + " bytes\n";
  payload += "Minimum Free Heap: " + String(minFreeHeap) + " bytes\n";
  payload += "Uptime: " + String(uptimeDays) + " days, ";
  payload += String(uptimeHours) + " hours, ";
  payload += String(uptimeMinutes) + " minutes, ";
  payload += String(uptimeSeconds) + " seconds";

  // Publish the system status to a specific topic
  if (safe_mqtt_publish("esp32/status", payload.c_str())) {
    Serial.println("System status published successfully.");
  } else {
    Serial.println("Failed to publish system status.");
  }
}


// *********************************************************
// MQTT Publish for All .dat files (Gateway and Slave)
// NOTE: Disabled - SD card and LoRa not used in this configuration
// *********************************************************
/*
void processFileTask(void * parameter) {
  while (true)
  {
    vTaskDelay(10000/portTICK_PERIOD_MS);
    // Process gateway data
    Serial.print("Processing mqtt for:"); Serial.println(systemConfig.DEVICE_NAME);
    mqtt_process_folder("/data", ".dat", systemConfig.DEVICE_NAME);

    // Process slave data
    for(int i = 0; i < peerCount; i++){
      String folerPath = "/node/" + String(peers[i].deviceName) + "/data";
      Serial.print("Processing mqtt for:"); Serial.println(String(peers[i].deviceName));
      mqtt_process_folder(folerPath, ".dat", String(peers[i].deviceName));
    }
  }
}
*/

// *********************************************************
// MQTT Publish for System Information
// *********************************************************
void systemInfoTask(void * parameter) {
  while (true){
    publish_system_status();
    vTaskDelay(60000/portTICK_PERIOD_MS);  // Publish status every 60 seconds
  }
}

// *********************************************************
// MQTT Keepalive Task - maintains connection
// *********************************************************
void mqttKeepaliveTask(void * parameter) {
  unsigned long lastReconnectAttempt = 0;
  const unsigned long reconnectInterval = 5000; // Try every 5 seconds
  
  while (true) {
    // Only try to reconnect if WiFi is connected
    if (WiFi.status() == WL_CONNECTED) {
      if (!client.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > reconnectInterval) {
          lastReconnectAttempt = now;
          mqtt_reconnect();
        }
      } else {
        // If connected, process MQTT messages
        client.loop();
      }
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);  // Check every second
  }
}

/******************************************************************
 *                                                                *
 *                       Initialization                           *
 *                                                                *
 ******************************************************************/

// *********************************************************
// Publish sensor data directly to MQTT (no SD card)
// *********************************************************
bool publish_sensor_data(int channel, const char* sensorType, float value, const char* timestamp) {
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();
  
  // Create JSON payload
  String topic = String(systemConfig.DEVICE_NAME) + "/sensor/" + String(channel);
  String payload = "{";
  payload += "\"device\":\"" + String(systemConfig.DEVICE_NAME) + "\",";
  payload += "\"channel\":" + String(channel) + ",";
  payload += "\"sensorType\":\"" + String(sensorType) + "\",";
  payload += "\"value\":" + String(value, 2) + ",";
  payload += "\"timestamp\":\"" + String(timestamp) + "\"";
  payload += "}";
  
  if (safe_mqtt_publish(topic.c_str(), payload.c_str())) {
    Serial.printf("Published: Channel %d, Value: %.2f\n", channel, value);
    return true;
  } else {
    Serial.printf("Failed to publish sensor data for channel %d\n", channel);
    return false;
  }
}

bool publish_srne_inverter_data(int channel, const char* timestamp) {
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();
  
  SRNEInverterData data;
  if (!read_srne_inverter_data(&data)) {
    Serial.printf("Channel %d: Failed to read SRNE inverter data\n", channel);
    return false;
  }
  
  // Create JSON payload with all inverter data
  String topic = String(systemConfig.DEVICE_NAME) + "/sensor/" + String(channel);
  String payload = "{";
  payload += "\"device\":\"" + String(systemConfig.DEVICE_NAME) + "\",";
  payload += "\"channel\":" + String(channel) + ",";
  payload += "\"sensorType\":\"SRNEInverter\",";
  payload += "\"timestamp\":\"" + String(timestamp) + "\",";
  payload += "\"data\":{";
  payload += "\"battery_soc\":" + String(data.battery_soc, 2) + ",";
  payload += "\"battery_voltage\":" + String(data.battery_voltage, 2) + ",";
  payload += "\"battery_current\":" + String(data.battery_current, 2) + ",";
  payload += "\"pv_voltage\":" + String(data.pv_voltage, 2) + ",";
  payload += "\"pv_current\":" + String(data.pv_current, 2) + ",";
  payload += "\"pv_power\":" + String(data.pv_power, 2) + ",";
  payload += "\"battery_charge_power\":" + String(data.battery_charge_power, 2) + ",";
  payload += "\"battery_type\":" + String(data.battery_type, 2) + ",";
  payload += "\"battery_over_voltage\":" + String(data.battery_over_voltage, 2) + ",";
  payload += "\"battery_equalizing_charge_voltage\":" + String(data.battery_equalizing_charge_voltage, 2) + ",";
  payload += "\"battery_boost_charge_voltage\":" + String(data.battery_boost_charge_voltage, 2) + ",";
  payload += "\"battery_float_charge_voltage\":" + String(data.battery_float_charge_voltage, 2) + ",";
  payload += "\"over_discharge_delay_time\":" + String(data.over_discharge_delay_time, 2) + ",";
  payload += "\"battery_equalizing_charge_time\":" + String(data.battery_equalizing_charge_time, 2) + ",";
  payload += "\"battery_equalizing_interval\":" + String(data.battery_equalizing_interval, 2) + ",";
  payload += "\"battery_under_voltage_warning\":" + String(data.battery_under_voltage_warning, 2) + ",";
  payload += "\"battery_over_discharge_voltage\":" + String(data.battery_over_discharge_voltage, 2) + ",";
  payload += "\"battery_limited_discharge_voltage\":" + String(data.battery_limited_discharge_voltage, 2) + ",";
  payload += "\"battery_boost_charge_time\":" + String(data.battery_boost_charge_time, 2) + ",";
  payload += "\"battery_mains_switching_voltage\":" + String(data.battery_mains_switching_voltage, 2) + ",";
  payload += "\"battery_stop_charging_current\":" + String(data.battery_stop_charging_current, 2) + ",";
  payload += "\"battery_number_in_series\":" + String(data.battery_number_in_series, 2) + ",";
  payload += "\"inverter_switch_voltage\":" + String(data.inverter_switch_voltage, 2) + ",";
  payload += "\"battery_max_charge_current\":" + String(data.battery_max_charge_current, 2) + ",";
  payload += "\"inverter_output_priority\":" + String(data.inverter_output_priority, 2) + ",";
  payload += "\"inverter_charge_priority\":" + String(data.inverter_charge_priority, 2) + ",";
  payload += "\"grid_battery_charge_max_current\":" + String(data.grid_battery_charge_max_current, 2) + ",";
  payload += "\"inverter_charger_priority\":" + String(data.inverter_charger_priority, 2) + ",";
  payload += "\"inverter_alarm_control\":" + String(data.inverter_alarm_control, 2) + ",";
  payload += "\"machine_state\":" + String(data.machine_state, 2) + ",";
  payload += "\"total_running_days\":" + String(data.total_running_days, 2) + ",";
  payload += "\"grid_voltage\":" + String(data.grid_voltage, 2) + ",";
  payload += "\"grid_input_current\":" + String(data.grid_input_current, 2) + ",";
  payload += "\"grid_frequency\":" + String(data.grid_frequency, 2) + ",";
  payload += "\"inverter_voltage\":" + String(data.inverter_voltage, 2) + ",";
  payload += "\"inverter_current\":" + String(data.inverter_current, 2) + ",";
  payload += "\"inverter_frequency\":" + String(data.inverter_frequency, 2) + ",";
  payload += "\"load_current\":" + String(data.load_current, 2) + ",";
  payload += "\"inverter_power\":" + String(data.inverter_power, 2) + ",";
  payload += "\"inverter_apparent_power\":" + String(data.inverter_apparent_power, 2) + ",";
  payload += "\"grid_battery_charge_current\":" + String(data.grid_battery_charge_current, 2) + ",";
  payload += "\"temp_dc\":" + String(data.temp_dc, 2) + ",";
  payload += "\"temp_ac\":" + String(data.temp_ac, 2) + ",";
  payload += "\"temp_tr\":" + String(data.temp_tr, 2) + ",";
  payload += "\"pv_battery_charge_current\":" + String(data.pv_battery_charge_current, 2);
  payload += "}";
  payload += "}";
  
  if (safe_mqtt_publish(topic.c_str(), payload.c_str())) {
    Serial.printf("Published SRNE Inverter data: Channel %d, Battery SOC: %.2f%%, Battery Voltage: %.2fV\n", 
                  channel, data.battery_soc, data.battery_voltage);
    return true;
  } else {
    Serial.printf("Failed to publish SRNE inverter data for channel %d\n", channel);
    return false;
  }
}

bool publish_single_phase_meter_data(int channel, const char* timestamp) {
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();
  
  SinglePhaseMeterData data;
  if (!read_single_phase_meter_data(&data)) {
    Serial.printf("Channel %d: Failed to read single phase meter data\n", channel);
    return false;
  }
  
  // Create JSON payload with all meter data
  String topic = String(systemConfig.DEVICE_NAME) + "/sensor/" + String(channel);
  String payload = "{";
  payload += "\"device\":\"" + String(systemConfig.DEVICE_NAME) + "\",";
  payload += "\"channel\":" + String(channel) + ",";
  payload += "\"sensorType\":\"SinglePhaseMeter\",";
  payload += "\"timestamp\":\"" + String(timestamp) + "\",";
  payload += "\"data\":{";
  payload += "\"voltage\":" + String(data.voltage, 2) + ",";
  payload += "\"current\":" + String(data.current, 2) + ",";
  payload += "\"frequency\":" + String(data.frequency, 2);
  payload += "}";
  payload += "}";
  
  if (safe_mqtt_publish(topic.c_str(), payload.c_str())) {
    Serial.printf("Published Single Phase Meter data: Channel %d, Voltage: %.2fV, Current: %.2fA, Frequency: %.2fHz\n", 
                  channel, data.voltage, data.current, data.frequency);
    return true;
  } else {
    Serial.printf("Failed to publish single phase meter data for channel %d\n", channel);
    return false;
  }
}

// Function to reinitialize MQTT with new server settings
void mqtt_reinit() {
  const char* mqtt_server = (strlen(systemConfig.MQTT_SERVER) > 0) ? systemConfig.MQTT_SERVER : mqtt_server_default;
  client.setServer(mqtt_server, 1883);
  Serial.printf("MQTT server updated to: %s\n", mqtt_server);
  // Disconnect and let reconnect task handle reconnection
  if (client.connected()) {
    client.disconnect();
  }
}

void mqtt_initialize() {
  // Get MQTT server from systemConfig or use default
  const char* mqtt_server = (strlen(systemConfig.MQTT_SERVER) > 0) ? systemConfig.MQTT_SERVER : mqtt_server_default;
  client.setServer(mqtt_server, 1883);
  client.setBufferSize(mqtt_buffer_size);
  Serial.printf("MQTT initialized with server: %s\n", mqtt_server);

  // Create a mutex for MQTT client access
  mqttMutex = xSemaphoreCreateMutex();

  // Create the task to process the file (only if SD card is available)
  // Commented out for no-SD-card mode
  // xTaskCreate(
  //   processFileTask,      // Task function
  //   "ProcessFileTask",    // Name of the task (for debugging)
  //   4 * 4096,                 // Stack size in words
  //   NULL,                 // Task input parameter
  //   1,                    // Priority of the task
  //   NULL                  // Task handle (can be NULL if not needed)
  // );

  // Create the task for system status publishing
  xTaskCreate(
    systemInfoTask,      // Task function
    "systemInfoTask",    // Name of the task (for debugging)
    4096,                 // Stack size in words
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    NULL                  // Task handle (can be NULL if not needed)
  );

  // Create the task to keep MQTT connection alive
  xTaskCreate(
    mqttKeepaliveTask,   // Task function
    "mqttKeepaliveTask", // Name of the task (for debugging)
    4096,                 // Stack size in words
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    NULL                  // Task handle (can be NULL if not needed)
  );
}