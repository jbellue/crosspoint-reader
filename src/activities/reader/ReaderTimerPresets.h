#pragma once

#include <array>
#include <cstdint>

#include <I18n.h>

#include "activities/ActivityResult.h"

namespace ReaderTimerPresets {

inline constexpr std::array<ReaderTimerConfigResult, 7> kTimeConfigs = {{
    {ReaderTimerMode::Time, 10},
    {ReaderTimerMode::Time, 5 * 60},
    {ReaderTimerMode::Time, 10 * 60},
    {ReaderTimerMode::Time, 15 * 60},
    {ReaderTimerMode::Time, 30 * 60},
    {ReaderTimerMode::Time, 60 * 60},
    {ReaderTimerMode::Chapter, 1},
}};

inline constexpr std::array<StrId, 7> kTimeLabelIds = {
    StrId::STR_SEC_10,
    StrId::STR_MIN_5,
    StrId::STR_MIN_10,
    StrId::STR_MIN_15,
    StrId::STR_MIN_30,
    StrId::STR_HOUR_1,
    StrId::STR_SNOOZE_END_CHAPTER
};

}  // namespace ReaderTimerPresets
