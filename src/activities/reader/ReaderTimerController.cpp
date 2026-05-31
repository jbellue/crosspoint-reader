#include "ReaderTimerController.h"

#include <CrossPointSettings.h>

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
  state.highWaterSpineIndex = currentSpineIndex;
  state.highWaterPage = currentPage;
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

void ReaderTimerController::recordForwardAdvance(const int newSpineIndex, const int newPage, const bool consumedPageStep,
                                                 const bool consumedChapterStep) {
  if (!consumedPageStep && !consumedChapterStep) {
    return;
  }

  if (!isPositionAfter(newSpineIndex, newPage, state.highWaterSpineIndex, state.highWaterPage)) {
    return;
  }

  state.highWaterSpineIndex = newSpineIndex;
  state.highWaterPage = newPage;

  if (consumedPageStep) {
    consumeTimerStep(ReaderTimerMode::Pages, 1);
  }
  if (consumedChapterStep) {
    consumeTimerStep(ReaderTimerMode::Chapters, 1);
  }
}

bool ReaderTimerController::isPositionAfter(const int spineA, const int pageA, const int spineB, const int pageB) {
  if (spineA != spineB) {
    return spineA > spineB;
  }
  return pageA > pageB;
}

bool ReaderTimerController::formatRemaining(char* out, const size_t outSize) const {
  if (!out || outSize == 0) {
    return false;
  }

  if (state.mode == ReaderTimerMode::Off || state.remaining == 0) {
    return false;
  }

  int n = 0;
  switch (state.mode) {
    case ReaderTimerMode::Time:
      if (state.remaining <= 60) {
        n = snprintf(out, outSize, "<1m");
      } else {
        n = snprintf(out, outSize, "%lum", static_cast<unsigned long>(state.remaining / 60));
      }
      break;
    case ReaderTimerMode::Pages:
      n = snprintf(out, outSize, "%lup", static_cast<unsigned long>(state.remaining));
      break;
    case ReaderTimerMode::Chapters:
      n = snprintf(out, outSize, "%luc", static_cast<unsigned long>(state.remaining));
      break;
    case ReaderTimerMode::Off:
    default:
      n = 0;
  }

  const bool success = (n >= 0 && static_cast<size_t>(n) < outSize);
  if (!success) {
    out[0] = '\0';
  }
  return success;
}

uint8_t ReaderTimerController::getStatusBarHeightForCurrentState() const {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const bool timerTextVisible =
      SETTINGS.statusBarTimerRemaining && state.mode != ReaderTimerMode::Off && state.remaining > 0;
  const bool showStatusBar = SETTINGS.statusBarChapterPageCount || SETTINGS.statusBarBookProgressPercentage ||
                             SETTINGS.statusBarTitle != CrossPointSettings::STATUS_BAR_TITLE::HIDE_TITLE ||
                             SETTINGS.statusBarBattery || timerTextVisible;
  return (showStatusBar ? metrics.statusBarVerticalMargin : 0) + UITheme::getInstance().getProgressBarHeight();
}
