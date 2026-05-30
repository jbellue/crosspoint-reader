#include "EpubReaderTimerActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "MappedInputManager.h"
#include "components/UITheme.h"

EpubReaderTimerActivity::EpubReaderTimerActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                         const ReaderTimerMode currentMode, const uint32_t currentValue,
                                                 const StrId screenTitleId, const bool includeOff,
                                                 const ReaderTimerMode customMode, const uint32_t customValue,
                                                 const std::string& customLabel)
    : Activity("EpubReaderTimer", renderer, mappedInput),
      options(buildOptions(includeOff, customMode, customValue, customLabel)),
      titleId(screenTitleId) {
  for (size_t i = 0; i < options.size(); i++) {
    const auto& option = options[i];
    if (option.mode == currentMode && option.value == currentValue) {
      selectedIndex = static_cast<int>(i);
      break;
    }
  }
}

std::vector<EpubReaderTimerActivity::TimerOption> EpubReaderTimerActivity::buildOptions(
    const bool includeOff, const ReaderTimerMode customMode, const uint32_t customValue, const std::string& customLabel) {
  std::vector<TimerOption> out;
  out.reserve(15);

  if (includeOff) {
    out.push_back({ReaderTimerMode::Off, 0, StrId::STR_TIMER_OFF, ""});
  }

  out.push_back({ReaderTimerMode::Time, 10, StrId::STR_SEC_10, ""});
  out.push_back({ReaderTimerMode::Time, 5 * 60, StrId::STR_MIN_5, ""});
  out.push_back({ReaderTimerMode::Time, 10 * 60, StrId::STR_MIN_10, ""});
  out.push_back({ReaderTimerMode::Time, 15 * 60, StrId::STR_MIN_15, ""});
  out.push_back({ReaderTimerMode::Time, 30 * 60, StrId::STR_MIN_30, ""});
  out.push_back({ReaderTimerMode::Time, 60 * 60, StrId::STR_HOUR_1, ""});

  out.push_back({ReaderTimerMode::Pages, 5, StrId::STR_PAGES_5, ""});
  out.push_back({ReaderTimerMode::Pages, 10, StrId::STR_PAGES_10, ""});
  out.push_back({ReaderTimerMode::Pages, 20, StrId::STR_PAGES_20, ""});
  out.push_back({ReaderTimerMode::Pages, 50, StrId::STR_PAGES_50, ""});

  out.push_back({ReaderTimerMode::Chapters, 1, StrId::STR_CHAPTERS_1, ""});
  out.push_back({ReaderTimerMode::Chapters, 2, StrId::STR_CHAPTERS_2, ""});
  out.push_back({ReaderTimerMode::Chapters, 3, StrId::STR_CHAPTERS_3, ""});

  if (!customLabel.empty() && customMode != ReaderTimerMode::Off && customValue > 0) {
    out.push_back({customMode, customValue, StrId::STR_TIMER, customLabel});
  }

  return out;
}

void EpubReaderTimerActivity::onEnter() {
  Activity::onEnter();
  requestUpdate();
}

void EpubReaderTimerActivity::onExit() { Activity::onExit(); }

void EpubReaderTimerActivity::loop() {
  buttonNavigator.onNext([this] {
    selectedIndex = ButtonNavigator::nextIndex(selectedIndex, static_cast<int>(options.size()));
    requestUpdate();
  });

  buttonNavigator.onPrevious([this] {
    selectedIndex = ButtonNavigator::previousIndex(selectedIndex, static_cast<int>(options.size()));
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
      renderer, Rect{screen.x, contentTop, screen.width, contentHeight}, options.size(), selectedIndex,
      [this](const int index) {
        if (!options[index].customLabel.empty()) {
          return options[index].customLabel;
        }
        return std::string(I18N.get(options[index].labelId));
      },
      nullptr, nullptr, nullptr, true);

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
