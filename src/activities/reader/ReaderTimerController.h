#pragma once

#include <cstddef>
#include <cstdint>

#include "activities/ActivityResult.h"

class ReaderTimerController {
 public:
  void reset();

  void applyTimerConfig(const ReaderTimerConfigResult& config, int currentSpineIndex, int currentPage);
  void applySnoozeConfig(const ReaderTimerConfigResult& config, int currentSpineIndex, int currentPage);

  void tickTimeTimer();
  void clearExpiryPromptPending() { state.expiryPromptPending = false; }
  bool hasExpiryPromptPending() const { return state.expiryPromptPending; }

  void recordForwardAdvance(int newSpineIndex, int newPage, bool consumedPageStep, bool consumedChapterStep);

  ReaderTimerConfigResult getSnoozeConfig() const { return {state.snoozeMode, state.snoozeValue}; }
  ReaderTimerMode getMode() const { return state.mode; }
  uint32_t getSelectedValue() const { return state.selectedValue; }
  const char* getSnoozeCustomLabel(uint32_t finishChapterPagesLeft, char* out, size_t outSize) const;

  bool formatRemaining(char* out, size_t outSize) const;

 private:
  struct State {
    ReaderTimerMode mode = ReaderTimerMode::Off;
    uint32_t remaining = 0;
    uint32_t selectedValue = 0;
    ReaderTimerMode snoozeMode = ReaderTimerMode::Time;
    uint32_t snoozeValue = 0;
    unsigned long lastTickMillis = 0UL;
    bool expiryPromptPending = false;
    int highWaterSpineIndex = -1;
    int highWaterPage = -1;
  } state;

  static bool isPositionAfter(int spineA, int pageA, int spineB, int pageB);
  void consumeTimerStep(ReaderTimerMode mode, uint32_t amount);
};
