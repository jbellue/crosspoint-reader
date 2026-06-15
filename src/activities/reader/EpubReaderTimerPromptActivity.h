#pragma once

#include <cstdint>
#include <string>

#include "activities/Activity.h"

class EpubReaderTimerPromptActivity final : public Activity {
 public:
  explicit EpubReaderTimerPromptActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("EpubReaderTimerPrompt", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  std::string safeHeading;
};
