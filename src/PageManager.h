#pragma once
#include <vector>
#include <memory>
#include "pages/Page.h"

// ============================================================
// PageManager — registry and cycling for 128×64 display pages.
// Pages render in registration order; nextPage() wraps around.
// ============================================================
class PageManager {
public:
    void  registerPage(std::unique_ptr<Page> page);
    void  nextPage();
    Page* activePage();

    int activeIndex() const { return _active; }
    int pageCount()   const { return static_cast<int>(_pages.size()); }

private:
    std::vector<std::unique_ptr<Page>> _pages;
    int _active = 0;
};
