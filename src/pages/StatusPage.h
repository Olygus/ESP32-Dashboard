#pragma once
#include "Page.h"

// Page 1 — Date · Time · Chi-Rho · Battery % · Remaining · Uptime
class StatusPage : public Page {
public:
    void        render(U8G2& display, const StatusData& status) override;
    const char* name() const override { return "Status"; }
};
