#include "LoRaLite.h"

// Simple stub globals
PeerInfo peers[8] = {};
int peerCount = 0;

uint8_t MAC_ADDRESS_STA[MAC_ADDR_LENGTH] = {0, 0, 0, 0, 0, 0};

LoRaConfig lora_config = {};

// Global LORA_MODE used elsewhere in the project (e.g. utils)
int LORA_MODE = LORA_GATEWAY;

// Very small handler registry – in the real library this would be more complex.
struct HandlerEntry {
  uint8_t msgType;
  LoRaMessageHandlerFunc handler;
};

static HandlerEntry handlers[8];
static int handlerCount = 0;

void lora_init(LoRaConfig *cfg) {
  // Stub: in the real implementation this would set up the LoRa radio
  (void)cfg;
  Serial.println("[LoRaLite stub] lora_init called");
}

int addSchedule(LoRaConfig *cfg, void (*func)(int), uint32_t intervalMs, int arg) {
  if (!cfg || cfg->scheduleCount >= 8) {
    return -1;
  }
  LoRaSchedule &s = cfg->schedules[cfg->scheduleCount++];
  s.func = func;
  s.intervalMs = intervalMs;
  s.lastRunMs = millis();
  s.arg = arg;
  Serial.println("[LoRaLite stub] addSchedule called");
  return 0;
}

int addHandler(LoRaConfig * /*cfg*/, uint8_t msgType, LoRaMessageHandlerFunc handler, void * /*userData*/) {
  if (handlerCount >= 8) {
    return -1;
  }
  handlers[handlerCount].msgType = msgType;
  handlers[handlerCount].handler = handler;
  ++handlerCount;
  Serial.println("[LoRaLite stub] addHandler called");
  return 0;
}

bool sendLoRaData(uint8_t *data, size_t len, const char *filename) {
  // Stub: just report the call, pretend success
  Serial.print("[LoRaLite stub] sendLoRaData called, bytes=");
  Serial.print(len);
  Serial.print(", file=");
  Serial.println(filename ? filename : "(null)");
  (void)data;
  return true;
}

void sendLoraMessage(uint8_t *data, size_t len) {
  // Stub: just report the call
  Serial.print("[LoRaLite stub] sendLoraMessage called, bytes=");
  Serial.println(len);
  (void)data;
}

void printMacAddress(const uint8_t *mac) {
  if (!mac) return;
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(buf);
}

void getMacByDeviceName(const String &name, uint8_t *mac) {
  // Simple linear search over stub peers
  for (int i = 0; i < peerCount; ++i) {
    if (name == String(peers[i].deviceName)) {
      memcpy(mac, peers[i].mac, MAC_ADDR_LENGTH);
      return;
    }
  }
  // Fallback – return zeros
  memset(mac, 0, MAC_ADDR_LENGTH);
}

int getIndexByMac(const uint8_t *mac) {
  for (int i = 0; i < peerCount; ++i) {
    if (memcmp(peers[i].mac, mac, MAC_ADDR_LENGTH) == 0) {
      return i;
    }
  }
  return -1;
}

bool isDeviceNameValid(const String &name) {
  // In stub mode, treat any name that matches an existing peer as valid.
  for (int i = 0; i < peerCount; ++i) {
    if (name == String(peers[i].deviceName)) {
      return true;
    }
  }
  return false;
}


