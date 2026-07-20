#include "EpubReaderTimerActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <cstring>

#include "ReaderTimerPresets.h"

EpubReaderTimerActivity::EpubReaderTimerActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                                 const ReaderTimerMode currentMode, const uint32_t currentValue,
                                                 const StrId screenTitleId, const bool includeOff,
                                                 const ReaderTimerMode customMode, const uint32_t customValue,
                                                 const char* customLabel)
  : Activity("EpubReaderTimer", renderer, mappedInput), titleId(screenTitleId) {

  const bool allowOffOption = includeOff && screenTitleId != StrId::STR_SNOOZE;
  if (allowOffOption && optionCount < MAX_OPTIONS) {
    options[optionCount++] = {ReaderTimerMode::Off, 0, StrId::STR_TIMER_OFF};
  }

  for (size_t i = 0; i < ReaderTimerPresets::kTimeConfigs.size(); ++i) {
    if (optionCount >= MAX_OPTIONS) {
      break;
    }
    const auto& preset = ReaderTimerPresets::kTimeConfigs[i];
    options[optionCount].mode = preset.mode;
    options[optionCount].value = preset.value;
    options[optionCount].labelId = ReaderTimerPresets::kTimeLabelIds[i];
    ++optionCount;
  }

  if (customLabel && customLabel[0] != '\0' && customMode != ReaderTimerMode::Off && customValue > 0 &&
      optionCount < MAX_OPTIONS) {
    strncpy(customOptionLabel, customLabel, sizeof(customOptionLabel) - 1);
    customOptionLabel[sizeof(customOptionLabel) - 1] = '\0';
    customOptionIndex = optionCount;
    options[optionCount++] = {customMode, customValue, StrId::STR_TIMER};
  }

  for (int i = 0; i < optionCount; i++) {
    const auto& option = options[i];
    if (option.mode == currentMode && option.value == currentValue) {
      selectedIndex = i;
      break;
    }
  }
}

void EpubReaderTimerActivity::onEnter() {
  Activity::onEnter();

  selectionCommitted = false;
  std::vector<std::string> popupOptions;
  popupOptions.reserve(optionCount);
  for (int i = 0; i < optionCount; i++) {
    if (i == customOptionIndex) {
      popupOptions.emplace_back(customOptionLabel);
    } else {
      popupOptions.emplace_back(I18N.get(options[i].labelId));
    }
  }

  optionPopup.show(titleId, popupOptions, selectedIndex, [this](int idx) {
    selectionCommitted = true;
    selectedIndex = idx;
    const auto& option = options[selectedIndex];
    setResult(ReaderTimerConfigResult{option.mode, option.value});
    finish();
  });

  requestUpdate();
}

void EpubReaderTimerActivity::onExit() { Activity::onExit(); }

void EpubReaderTimerActivity::loop() {
  const bool wasActive = optionPopup.isActive();
  if (optionPopup.handleInput(mappedInput, [this] { requestUpdate(); })) {
    if (wasActive && !optionPopup.isActive() && !selectionCommitted) {
      // Back closes OptionPopup without a selection callback.
      ActivityResult result;
      result.isCancelled = true;
      setResult(std::move(result));
      finish();
    }
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    ActivityResult result;
    result.isCancelled = true;
    setResult(std::move(result));
    finish();
    return;
  }
}

void EpubReaderTimerActivity::render(RenderLock&&) { if (optionPopup.processRender(renderer, mappedInput)) return; }
