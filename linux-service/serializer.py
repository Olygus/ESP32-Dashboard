"""
serializer.py — StatusData dataclass and JSON serialisation.

The JSON produced by to_json() is consumed by StatusManager::updateFromJson()
on the ESP32.  Key names here must match the C++ code exactly.
Payload is kept compact (no spaces) to stay well within the 512-byte buffer.
"""
from __future__ import annotations

import json
from dataclasses import dataclass
from typing import Optional


@dataclass
class StatusData:
    date:             str
    time:             str
    uptime:           str
    batteryPercent:   int
    batteryRemaining: str
    stylusPercent:    Optional[int] = None   # None → JSON null → ESP32 reads -1

    def to_json(self) -> str:
        payload = {
            "date":             self.date,
            "time":             self.time,
            "uptime":           self.uptime,
            "batteryPercent":   self.batteryPercent,
            "batteryRemaining": self.batteryRemaining,
            "stylusPercent":    self.stylusPercent,
        }
        return json.dumps(payload, separators=(",", ":"))
