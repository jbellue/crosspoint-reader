#pragma once

#include <array>
#include <cstdint>

#include <I18n.h>

#include "MappedInputManager.h"
#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class EpubReaderTimerActivity final : public Activity {
 public:
  EpubReaderTimerActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, ReaderTimerMode currentMode,
                          uint32_t currentValue, StrId screenTitleId = StrId::STR_TIMER, bool includeOff = true,
                          ReaderTimerMode customMode = ReaderTimerMode::Off, uint32_t customValue = 0,
                          const char* customLabel = nullptr);

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  struct TimerOption {
    ReaderTimerMode mode;
    uint32_t value;
    StrId labelId;
  };

  static constexpr int MAX_OPTIONS = 15;
  static constexpr size_t MAX_CUSTOM_LABEL_LEN = 96;

  std::array<TimerOption, MAX_OPTIONS> options = {};
  int optionCount = 0;
  int customOptionIndex = -1;
  char customOptionLabel[MAX_CUSTOM_LABEL_LEN] = {};
  StrId titleId = StrId::STR_TIMER;
  int selectedIndex = 0;
  ButtonNavigator buttonNavigator;
};
