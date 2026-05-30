#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <I18n.h>

#include "MappedInputManager.h"
#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class EpubReaderTimerActivity final : public Activity {
 public:
  EpubReaderTimerActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, ReaderTimerMode currentMode,
                          uint32_t currentValue, StrId screenTitleId = StrId::STR_TIMER, bool includeOff = true,
                          ReaderTimerMode customMode = ReaderTimerMode::Off, uint32_t customValue = 0,
                          const std::string& customLabel = "");

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  struct TimerOption {
    ReaderTimerMode mode;
    uint32_t value;
    StrId labelId;
    std::string customLabel;
  };

  static std::vector<TimerOption> buildOptions(bool includeOff, ReaderTimerMode customMode, uint32_t customValue,
                                               const std::string& customLabel);

  const std::vector<TimerOption> options;
  StrId titleId = StrId::STR_TIMER;
  int selectedIndex = 0;
  ButtonNavigator buttonNavigator;
};
