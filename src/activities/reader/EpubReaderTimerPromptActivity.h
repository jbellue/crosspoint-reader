#pragma once

#include <cstdint>

#include "activities/Activity.h"
#include "components/OptionPopup.h"

class EpubReaderTimerPromptActivity final : public Activity {
 public:
  explicit EpubReaderTimerPromptActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("EpubReaderTimerPrompt", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  static constexpr int OPTION_SNOOZE = 0;
  static constexpr int OPTION_SLEEP = 1;
  static constexpr int OPTION_CANCEL = 2;

  static constexpr uint32_t ACTION_CANCEL = 0;
  static constexpr uint32_t ACTION_SNOOZE = 1;
  static constexpr uint32_t ACTION_SLEEP = 2;

  OptionPopup optionPopup;
  bool selectionCommitted = false;
};
