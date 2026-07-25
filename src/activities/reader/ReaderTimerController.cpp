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

std::string ReaderTimerController::formatRemaining(bool compact) const {
  if (state.mode == ReaderTimerMode::Off || state.remaining == 0) {
    return {};
  }

  if (state.mode == ReaderTimerMode::Time) {
    if (state.remaining < 60) {
        return compact ? tr(STR_TIMER_LESS_THAN_ONE_MIN_SHORT) : tr(STR_TIMER_LESS_THAN_ONE_MIN);
    }

    const char* fmt = compact ? tr(STR_TIMER_MINUTES_SHORT_FORMAT) : tr(STR_TIMER_MINUTES_FORMAT);
    const auto value = static_cast<unsigned long>(state.remaining / 60);

    const int n = std::snprintf(nullptr, 0, fmt, value);
    std::string s;
    if (n > 0) {
        s.resize(static_cast<size_t>(n));
        std::snprintf(s.data(), s.size() + 1, fmt, value);
    }
    return s;
  }

  if (state.mode == ReaderTimerMode::Chapter) {
    return tr(STR_TIMER_END_OF_CHAPTER);
  }

  return {};
}

