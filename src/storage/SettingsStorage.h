#pragma once

#include <Preferences.h>

#include "core/AppTypes.h"

// Low-level NVS persistence. Stores one versioned, checksummed blob so a
// corrupted or obsolete record is rejected as a whole instead of being applied
// partially.
class SettingsStorage {
 public:
  bool begin();
  bool load(AppSettings& out);
  bool save(const AppSettings& settings);
  void clear();

 private:
  Preferences prefs_;
  bool ready_ = false;
};
