"""
time_collector.py — current date and time.
"""
from __future__ import annotations

from datetime import datetime


def collect() -> dict:
    now = datetime.now()
    return {
        "date": now.strftime("%Y-%m-%d"),
        "time": now.strftime("%H:%M"),
    }
