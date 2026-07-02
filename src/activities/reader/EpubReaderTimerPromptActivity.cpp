#include "EpubReaderTimerPromptActivity.h"

void EpubReaderTimerPromptActivity::onEnter() {
  Activity::onEnter();

  selectionCommitted = false;
  const char* const options[] = {tr(STR_SNOOZE), tr(STR_SLEEP), tr(STR_CANCEL)};
  optionPopup.show(tr(STR_TIMER_EXPIRED_TITLE), options, 3, OPTION_SNOOZE, [this](int idx) {
    selectionCommitted = true;
    IntervalResult actionResult;
    if (idx == OPTION_SLEEP) {
      actionResult.value = ACTION_SLEEP;
    } else if (idx == OPTION_SNOOZE) {
      actionResult.value = ACTION_SNOOZE;
    } else {
      actionResult.value = ACTION_CANCEL;
    }
    setResult(actionResult);
    finish();
  });

  requestUpdate(true);
}

void EpubReaderTimerPromptActivity::loop() {
  const bool wasActive = optionPopup.isActive();
  if (optionPopup.handleInput(mappedInput, [this] { requestUpdate(); })) {
    if (wasActive && !optionPopup.isActive() && !selectionCommitted) {
      // Back closes OptionPopup without a selection callback; treat as Cancel.
      setResult(IntervalResult{ACTION_CANCEL});
      finish();
    }
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    // Safety fallback in case popup is not active.
    setResult(IntervalResult{ACTION_CANCEL});
    finish();
    return;
  }
}

void EpubReaderTimerPromptActivity::render(RenderLock&&) {
  if (optionPopup.processRender(renderer, mappedInput)) return;
}
