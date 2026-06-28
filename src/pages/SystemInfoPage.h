#pragma once
#include "Page.h"

// Page 2 — Battery % · Time remaining · Last update age
class SystemInfoPage : public Page {
public:
    void        render(U8G2& display, const StatusData& status) override;
    const char* name() const override { return "System Info"; }
};
