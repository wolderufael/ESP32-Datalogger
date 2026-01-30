#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
// #include <SD.h>  // Disabled - no SD card needed
#include "configuration.h"
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