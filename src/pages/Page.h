#pragma once
#include <U8g2lib.h>
#include "StatusManager.h"

// ============================================================
// Page — abstract interface for a 128×64 display page.
// Subclass, implement render() + name(), then register with
// pageMgr.registerPage(std::make_unique<MyPage>()) in setup().
// ============================================================
class Page {
public:
    virtual ~Page() = default;
    virtual void        render(U8G2& display, const StatusData& status) = 0;
    virtual const char* name() const = 0;
};
