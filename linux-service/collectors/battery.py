"""
battery.py — laptop battery info from /sys/class/power_supply.
"""
from __future__ import annotations

import logging
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

log = logging.getLogger("collectors.battery")

SUPPLY_ROOT = Path("/sys/class/power_supply")


@dataclass
class BatteryInfo:
    percent:   int
    remaining: str   # e.g. "2:30" or "" if unknown


def collect() -> BatteryInfo:
    """Find the system battery and return percent + remaining time."""
    # Prefer well-known names, then fall back to type-sniffing.
    for name in ("BAT0", "BAT1", "BAT"):
        p = SUPPLY_ROOT / name
        if p.exists():
            return _read(p)

    for p in SUPPLY_ROOT.iterdir():
        try:
            if (p / "type").read_text().strip() == "Battery":
                return _read(p)
        except OSError:
            pass

    log.warning("No battery found under %s.", SUPPLY_ROOT)
    return BatteryInfo(percent=0, remaining="")


# ------------------------------------------------------------------ #

def _read(path: Path) -> BatteryInfo:
    def rd(name: str) -> Optional[str]:
        try:
            return (path / name).read_text().strip()
        except OSError:
            return None

    percent   = int(rd("capacity") or 0)
    remaining = _time_remaining(path, rd)
    return BatteryInfo(percent=percent, remaining=remaining)


def _time_remaining(path: Path, rd) -> str:
    status = rd("status") or ""

    # Strategy 1: energy_now (µWh) / power_now (µW) = hours
    try:
        energy_now = int(rd("energy_now") or 0)
        power_now  = int(rd("power_now")  or 0)
        if power_now > 0:
            if "Discharging" in status:
                hours = energy_now / power_now
            elif "Charging" in status:
                energy_full = int(rd("energy_full") or 0)
                hours = (energy_full - energy_now) / power_now
            else:
                return ""
            return _fmt(hours)
    except (ValueError, ZeroDivisionError):
        pass

    # Strategy 2: charge_now (µAh) / current_now (µA) = hours
    try:
        charge_now  = int(rd("charge_now")  or 0)
        current_now = int(rd("current_now") or 0)
        if current_now > 0:
            if "Discharging" in status:
                hours = charge_now / current_now
            elif "Charging" in status:
                charge_full = int(rd("charge_full") or 0)
                hours = (charge_full - charge_now) / current_now
            else:
                return ""
            return _fmt(hours)
    except (ValueError, ZeroDivisionError):
        pass

    return ""


def _fmt(hours: float) -> str:
    if hours <= 0:
        return ""
    h = int(hours)
    m = int((hours - h) * 60)
    return f"{h}:{m:02d}"
