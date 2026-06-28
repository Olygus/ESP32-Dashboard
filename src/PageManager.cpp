#include "PageManager.h"

void PageManager::registerPage(std::unique_ptr<Page> page) {
    Serial.printf("[Pages] Registered: %s\n", page->name());
    _pages.push_back(std::move(page));
}

void PageManager::nextPage() {
    if (_pages.empty()) return;
    _active = (_active + 1) % static_cast<int>(_pages.size());
    Serial.printf("[Pages] Active: %s (%d/%d)\n",
                  _pages[_active]->name(),
                  _active + 1,
                  static_cast<int>(_pages.size()));
}

Page* PageManager::activePage() {
    if (_pages.empty()) return nullptr;
    return _pages[_active].get();
}
