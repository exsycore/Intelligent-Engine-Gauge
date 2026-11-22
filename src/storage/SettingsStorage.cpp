#include "storage/SettingsStorage.h"

#include <Arduino.h>
#include <string.h>

namespace {

constexpr const char* kNamespace = "smartgauge";
constexpr const char* kKey = "cfg";
constexpr uint16_t kMagic = 0x5A47;  // "ZG"
constexpr uint8_t kVersion = 1;

struct __attribute__((packed)) PersistedSettings {
  uint16_t magic;
  uint8_t version;
  uint8_t payloadSize;
  AppSettings data;
  uint16_t checksum;
};

uint16_t crc16(const uint8_t* data, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; ++i) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x8000) ? static_cast<uint16_t>((crc << 1) ^ 0x1021)
                           : static_cast<uint16_t>(crc << 1);
    }
  }
  return crc;
}

}  // namespace

bool SettingsStorage::begin() {
  ready_ = prefs_.begin(kNamespace, false);
  return ready_;
}

bool SettingsStorage::load(AppSettings& out) {
  if (!ready_) return false;
  if (!prefs_.isKey(kKey)) return false;
  if (prefs_.getBytesLength(kKey) != sizeof(PersistedSettings)) return false;

  PersistedSettings record;
  memset(&record, 0, sizeof(record));
  size_t read = prefs_.getBytes(kKey, &record, sizeof(record));
  if (read != sizeof(record)) return false;

  if (record.magic != kMagic || record.version != kVersion ||
      record.payloadSize != sizeof(AppSettings)) {
    return false;
  }

  uint16_t expected = crc16(reinterpret_cast<const uint8_t*>(&record.data), sizeof(AppSettings));
  if (record.checksum != expected) return false;

  out = record.data;
  return true;
}

bool SettingsStorage::save(const AppSettings& settings) {
  if (!ready_) return false;

  PersistedSettings record;
  memset(&record, 0, sizeof(record));
  record.magic = kMagic;
  record.version = kVersion;
  record.payloadSize = sizeof(AppSettings);
  record.data = settings;
  record.checksum =
      crc16(reinterpret_cast<const uint8_t*>(&record.data), sizeof(AppSettings));

  size_t written = prefs_.putBytes(kKey, &record, sizeof(record));
  return written == sizeof(record);
}

void SettingsStorage::clear() {
  if (ready_) prefs_.remove(kKey);
}
