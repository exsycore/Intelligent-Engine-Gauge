#include "ui/ScreenManager.h"

namespace {
constexpr uint8_t kScreenCount = static_cast<uint8_t>(ScreenId::Count);
}

void ScreenManager::add(ScreenId id, Screen* screen) {
  uint8_t index = static_cast<uint8_t>(id);
  if (index >= kScreenCount) return;
  screens_[index] = screen;
}

void ScreenManager::start(ScreenId id) {
  uint8_t index = static_cast<uint8_t>(id);
  if (index >= kScreenCount || screens_[index] == nullptr) return;
  currentId_ = id;
  current_ = screens_[index];
  stackDepth_ = 0;
  current_->enter();
}

void ScreenManager::show(ScreenId id) {
  uint8_t index = static_cast<uint8_t>(id);
  if (index >= kScreenCount || screens_[index] == nullptr) return;
  if (current_ != nullptr && stackDepth_ < kMaxStack) {
    stack_[stackDepth_++] = currentId_;
  }
  currentId_ = id;
  current_ = screens_[index];
  current_->enter();
}

void ScreenManager::back() {
  if (stackDepth_ == 0) {
    show(ScreenId::Dashboard);
    return;
  }
  ScreenId previous = stack_[--stackDepth_];
  uint8_t index = static_cast<uint8_t>(previous);
  if (screens_[index] == nullptr) {
    show(ScreenId::Dashboard);
    return;
  }
  currentId_ = previous;
  current_ = screens_[index];
  current_->enter();
}

void ScreenManager::redraw() {
  if (current_ != nullptr) current_->enter();
}

Screen& ScreenManager::current() { return *current_; }
