#pragma once

#include <array>
#include <cstdint>

#include <I18n.h>

#include "activities/ActivityResult.h"

namespace ReaderTimerPresets {

inline constexpr std::array<ReaderTimerConfigResult, 6> kTimeConfigs = {{
    {ReaderTimerMode::Time, 10},
    {ReaderTimerMode::Time, 5 * 60},
    {ReaderTimerMode::Time, 10 * 60},
    {ReaderTimerMode::Time, 15 * 60},
    {ReaderTimerMode::Time, 30 * 60},
    {ReaderTimerMode::Time, 60 * 60},
}};

inline constexpr std::array<StrId, 6> kTimeLabelIds = {
    StrId::STR_SEC_10,
    StrId::STR_MIN_5,
    StrId::STR_MIN_10,
    StrId::STR_MIN_15,
    StrId::STR_MIN_30,
    StrId::STR_HOUR_1,
};

constexpr uint8_t optionIndexFromConfig(const ReaderTimerMode mode, const uint32_t value) {
  for (size_t i = 0; i < kTimeConfigs.size(); ++i) {
    if (kTimeConfigs[i].mode == mode && kTimeConfigs[i].value == value) {
      // Option index 0 is reserved for "Cancel" in the reader menu popup.
      return static_cast<uint8_t>(i + 1);
    }
  }
  return 0;
}

constexpr ReaderTimerConfigResult configFromOptionIndex(const uint8_t optionIndex) {
  if (optionIndex == 0) {
    return {ReaderTimerMode::Off, 0};
  }

  const size_t presetIndex = static_cast<size_t>(optionIndex - 1);
  if (presetIndex >= kTimeConfigs.size()) {
    const auto& fallback = kTimeConfigs[kTimeConfigs.size() - 1];
    return {fallback.mode, fallback.value};
  }

  const auto& preset = kTimeConfigs[presetIndex];
  return {preset.mode, preset.value};
}

}  // namespace ReaderTimerPresets
