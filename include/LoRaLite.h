#ifndef LORALITE_H
#define LORALITE_H

#include <Arduino.h>
#include <time.h>

#include "configuration.h"

// Basic LoRaLite stubs so the project can compile without the external library.
// These provide just enough structure for the existing code to link; they do
// NOT implement real LoRa radio behaviour.

#define MAC_ADDR_LENGTH 6

// Simple LoRa mode definitions used by the configuration code
enum LoRaMode : int {
  LORA_NODE = 0,
  LORA_GATEWAY = 1
};

// Message types used in this project
enum LoRaMessageType : uint8_t {
  TIME_SYNC = 1,
  GET_CONFIG = 2,
  UPDATE_DATA_CONFIG = 3,
  UPDATE_SYS_CONFIG = 4,
  SYNC_FOLDER = 5,
  POLL_COMPLETE = 6
};

struct PeerInfo {
  uint8_t mac[MAC_ADDR_LENGTH];
  char deviceName[16];
  struct tm lastCommTime;
  int status;
  int SignalStrength;
};

// Simple globals representing the LoRa network peers
extern PeerInfo peers[8];
extern int peerCount;

// Station MAC (stubbed)
extern uint8_t MAC_ADDRESS_STA[MAC_ADDR_LENGTH];

// Application message structures (minimal fields used by this project)
struct time_sync_message {
  uint8_t msgType;
  uint32_t pairingKey;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

struct collection_config_message {
  uint8_t msgType;
  uint8_t mac[MAC_ADDR_LENGTH];
  int channel;
  int pin;
  SensorType sensor;
  bool enabled;
  uint16_t interval;
};

// Length for sysconfig key/value strings
static const size_t MAX_JSON_LEN_1 = 64;

struct sysconfig_message {
  uint8_t msgType;
  uint8_t mac[MAC_ADDR_LENGTH];
  char key[MAX_JSON_LEN_1];
  char value[MAX_JSON_LEN_1];
};

struct signal_message {
  uint8_t msgType;
  uint8_t mac[MAC_ADDR_LENGTH];
  char path[32];
  char extension[8];
};

// Handler function type
typedef void (*LoRaMessageHandlerFunc)(const uint8_t *data);

struct LoRaSchedule {
  void (*func)(int index);
  uint32_t intervalMs;
  uint32_t lastRunMs;
  int arg;
};

struct LoRaConfig {
  int lora_mode;
  uint32_t pairingKey;
  char deviceName[16];
  LoRaSchedule schedules[8];
  int scheduleCount;
};

extern LoRaConfig lora_config;

// Stub API functions
void lora_init(LoRaConfig *cfg);
int addSchedule(LoRaConfig *cfg, void (*func)(int), uint32_t intervalMs, int arg);
int addHandler(LoRaConfig *cfg, uint8_t msgType, LoRaMessageHandlerFunc handler, void *userData);
bool sendLoRaData(uint8_t *data, size_t len, const char *filename);
void sendLoraMessage(uint8_t *data, size_t len);
void printMacAddress(const uint8_t *mac);
void getMacByDeviceName(const String &name, uint8_t *mac);
int getIndexByMac(const uint8_t *mac);
bool isDeviceNameValid(const String &name);

#endif


