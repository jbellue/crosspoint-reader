#include "EpubReaderTimerPromptActivity.h"

#include <HalDisplay.h>
#include <HalGPIO.h>
#include <I18n.h>

#include "components/UITheme.h"
#include "fontIds.h"

void EpubReaderTimerPromptActivity::onEnter() {
  Activity::onEnter();

  const int maxWidth = renderer.getScreenWidth() - 40;
  safeHeading = renderer.truncatedText(UI_10_FONT_ID, tr(STR_TIMER_EXPIRED_TITLE), maxWidth, EpdFontFamily::BOLD);

  requestUpdate(true);
}

void EpubReaderTimerPromptActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    ActivityResult result;
    result.isCancelled = false;
    setResult(std::move(result));
    finish();
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    ActivityResult result;
    result.isCancelled = true;
    setResult(std::move(result));
    finish();
    return;
  }
}

void EpubReaderTimerPromptActivity::render(RenderLock&&) {
  GUI.drawPopup(renderer, safeHeading.c_str());

  GUI.drawButtonHints(renderer, tr(STR_SNOOZE), tr(STR_SLEEP), "", "");

  renderer.displayBuffer(HalDisplay::RefreshMode::HALF_REFRESH);
}
