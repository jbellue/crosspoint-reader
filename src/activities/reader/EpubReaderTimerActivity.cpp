#include "EpubReaderTimerActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <cstring>

#include "MappedInputManager.h"
#include "components/UITheme.h"

EpubReaderTimerActivity::EpubReaderTimerActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                                 const ReaderTimerMode currentMode, const uint32_t currentValue,
                                                 const StrId screenTitleId, const bool includeOff,
                                                 const ReaderTimerMode customMode, const uint32_t customValue,
                                                 const char* customLabel)
    : Activity("EpubReaderTimer", renderer, mappedInput), titleId(screenTitleId) {
  constexpr std::array<TimerOption, 13> timerPresets = {{
      {ReaderTimerMode::Time, 10, StrId::STR_SEC_10},
      {ReaderTimerMode::Time, 5 * 60, StrId::STR_MIN_5},
      {ReaderTimerMode::Time, 10 * 60, StrId::STR_MIN_10},
      {ReaderTimerMode::Time, 15 * 60, StrId::STR_MIN_15},
      {ReaderTimerMode::Time, 30 * 60, StrId::STR_MIN_30},
      {ReaderTimerMode::Time, 60 * 60, StrId::STR_HOUR_1},
      {ReaderTimerMode::Pages, 5, StrId::STR_PAGES_5},
      {ReaderTimerMode::Pages, 10, StrId::STR_PAGES_10},
      {ReaderTimerMode::Pages, 20, StrId::STR_PAGES_20},
      {ReaderTimerMode::Pages, 50, StrId::STR_PAGES_50},
      {ReaderTimerMode::Chapters, 1, StrId::STR_CHAPTERS_1},
      {ReaderTimerMode::Chapters, 2, StrId::STR_CHAPTERS_2},
      {ReaderTimerMode::Chapters, 3, StrId::STR_CHAPTERS_3},
  }};

  if (includeOff && optionCount < MAX_OPTIONS) {
    options[optionCount++] = {ReaderTimerMode::Off, 0, StrId::STR_TIMER_OFF};
  }

  for (const auto& preset : timerPresets) {
    if (optionCount >= MAX_OPTIONS) {
      break;
    }
    options[optionCount++] = preset;
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
  requestUpdate();
}

void EpubReaderTimerActivity::onExit() { Activity::onExit(); }

void EpubReaderTimerActivity::loop() {
  buttonNavigator.onNext([this] {
    selectedIndex = ButtonNavigator::nextIndex(selectedIndex, optionCount);
    requestUpdate();
  });

  buttonNavigator.onPrevious([this] {
    selectedIndex = ButtonNavigator::previousIndex(selectedIndex, optionCount);
    requestUpdate();
  });

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    ActivityResult result;
    result.isCancelled = true;
    setResult(std::move(result));
    finish();
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    const auto& option = options[selectedIndex];
    setResult(ReaderTimerConfigResult{option.mode, option.value});
    finish();
    return;
  }
}

void EpubReaderTimerActivity::render(RenderLock&&) {
  renderer.clearScreen();

  auto metrics = UITheme::getInstance().getMetrics();
  Rect screen = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
  const StrId headerTitleId = this->titleId;

  GUI.drawHeader(renderer, Rect{screen.x, screen.y + metrics.topPadding, screen.width, metrics.headerHeight},
                 I18n::getInstance().get(headerTitleId));

  const int contentTop = screen.y + metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int contentHeight = screen.height - contentTop - metrics.verticalSpacing;

  GUI.drawList(
      renderer, Rect{screen.x, contentTop, screen.width, contentHeight}, optionCount, selectedIndex,
      [this](const int index) {
        if (index == customOptionIndex) {
          return std::string(customOptionLabel);
        }
        return std::string(I18N.get(options[index].labelId));
      },
      nullptr, nullptr, nullptr, true);

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
