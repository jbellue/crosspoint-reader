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

void ReaderTimerController::recordForwardAdvance(const int newSpineIndex, const int newPage, const bool consumedPageStep) {
  if (!consumedPageStep) {
    return;
  }

  if (!isPositionAfter(newSpineIndex, newPage, state.highWaterSpineIndex, state.highWaterPage)) {
    return;
  }

  state.highWaterSpineIndex = newSpineIndex;
  state.highWaterPage = newPage;

  consumeTimerStep(ReaderTimerMode::Pages, 1);
}

const char* ReaderTimerController::getSnoozeCustomLabel(const uint32_t finishChapterPagesLeft, char* out,
                                                        const size_t outSize) const {
  if (!out || outSize == 0 || finishChapterPagesLeft == 0) {
    return nullptr;
  }

  const char* baseLabel = tr(STR_SNOOZE_END_CHAPTER);
  int n = snprintf(out, outSize, "%s", baseLabel);
  if (n < 0 || static_cast<size_t>(n) >= outSize) {
    out[0] = '\0';
    return nullptr;
  }

  if (finishChapterPagesLeft <= 1) {
    return out;
  }

  size_t used = static_cast<size_t>(n);
  if (used + 1 >= outSize) {
    out[0] = '\0';
    return nullptr;
  }
  out[used++] = ' ';
  out[used] = '\0';

  n = snprintf(out + used, outSize - used, tr(STR_SNOOZE_PAGES_LEFT_FORMAT), static_cast<int>(finishChapterPagesLeft));
  if (n < 0 || static_cast<size_t>(n) >= (outSize - used)) {
    out[0] = '\0';
    return nullptr;
  }

  return out;
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
      if (state.remaining < 60) {
        n = snprintf(out, outSize, "%s", tr(STR_TIMER_LESS_THAN_ONE_MIN));
      } else {
        n = snprintf(out, outSize, tr(STR_TIMER_MINUTES_FORMAT), static_cast<unsigned long>(state.remaining / 60));
      }
      break;
    case ReaderTimerMode::Pages:
      n = snprintf(out, outSize, tr(STR_TIMER_PAGES_SHORT_FORMAT), static_cast<unsigned long>(state.remaining));
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

