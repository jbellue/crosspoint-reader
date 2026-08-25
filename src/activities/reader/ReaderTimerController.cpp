#include "ReaderTimerController.h"

#include <CrossPointSettings.h>
#include <I18n.h>

#include <cstdio>

#include "components/UITheme.h"

namespace {
constexpr uint32_t TIMER_SNOOZE_SECONDS = 10UL * 60UL;
}

void ReaderTimerController::reset() {
  state = State{};
  state.snoozeMode = ReaderTimerMode::Time;
  state.snoozeValue = TIMER_SNOOZE_SECONDS;
}

void ReaderTimerController::applyTimerConfig(const ReaderTimerConfigResult& config, const int currentSpineIndex,
                                             const int currentPage) {
  state.mode = config.mode;
  state.selectedValue = config.value;
  state.remaining = config.value;
  state.lastTickMillis = millis();
  state.expiryPromptPending = false;
  state.currentSpineIndex = currentSpineIndex;
}

void ReaderTimerController::applySnoozeConfig(const ReaderTimerConfigResult& config, const int currentSpineIndex,
                                              const int currentPage) {
  if (config.mode == ReaderTimerMode::Off || config.value == 0) {
    return;
  }

  state.snoozeMode = config.mode;
  state.snoozeValue = config.value;
  applyTimerConfig(config, currentSpineIndex, currentPage);
}

void ReaderTimerController::tickTimeTimer() {
  if (state.mode != ReaderTimerMode::Time || state.remaining == 0 || state.expiryPromptPending) {
    return;
  }

  const unsigned long now = millis();
  if (state.lastTickMillis == 0UL) {
    state.lastTickMillis = now;
    return;
  }

  const unsigned long elapsedMs = now - state.lastTickMillis;
  if (elapsedMs < 1000UL) {
    return;
  }

  const uint32_t elapsedSeconds = static_cast<uint32_t>(elapsedMs / 1000UL);
  if (elapsedSeconds == 0) {
    return;
  }

  if (elapsedSeconds >= state.remaining) {
    state.remaining = 0;
    state.expiryPromptPending = true;
  } else {
    state.remaining -= elapsedSeconds;
  }
  state.lastTickMillis = now;
}

void ReaderTimerController::consumeTimerStep(const ReaderTimerMode mode, const uint32_t amount) {
  if (state.mode != mode || amount == 0 || state.remaining == 0 || state.expiryPromptPending) {
    return;
  }

  if (amount >= state.remaining) {
    state.remaining = 0;
    state.expiryPromptPending = true;
  } else {
    state.remaining -= amount;
  }
}

void ReaderTimerController::recordForwardAdvance(const int newSpineIndex, const int newPage, const bool consumedPageStep) {
  if (!consumedPageStep) {
    return;
  }

  if (newSpineIndex <= state.currentSpineIndex) {
    return;
  }

  state.currentSpineIndex = newSpineIndex;
  consumeTimerStep(ReaderTimerMode::Chapter, 1);
}

bool ReaderTimerController::formatRemaining(char* buffer, size_t bufferSize, bool compact) const {
  if (buffer == nullptr || bufferSize == 0) {
    return false;
  }

  buffer[0] = '\0';

  if (state.mode == ReaderTimerMode::Off || state.remaining == 0) {
    return false;
  }

  if (state.mode == ReaderTimerMode::Time) {
    if (state.remaining < 60) {
      const char* text = compact ? tr(STR_TIMER_LESS_THAN_ONE_MIN_SHORT) : tr(STR_TIMER_LESS_THAN_ONE_MIN);
      return std::snprintf(buffer, bufferSize, "%s", text) >= 0;
    }

    const char* fmt = compact ? tr(STR_TIMER_MINUTES_SHORT_FORMAT) : tr(STR_TIMER_MINUTES_FORMAT);
    const auto value = static_cast<unsigned long>(state.remaining / 60);
    return std::snprintf(buffer, bufferSize, fmt, value) >= 0;
  }

  if (state.mode == ReaderTimerMode::Chapter) {
    const char* text = tr(STR_TIMER_END_OF_CHAPTER);
    return std::snprintf(buffer, bufferSize, "%s", text) >= 0;
  }

  return false;
}

