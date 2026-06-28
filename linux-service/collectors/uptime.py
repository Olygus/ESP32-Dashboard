"""
uptime.py — system uptime from /proc/uptime.
"""
from __future__ import annotations

import logging
from pathlib import Path

log = logging.getLogger("collectors.uptime")


def collect() -> str:
    """Return uptime as a human-readable string, e.g. '3d 4h 22m'."""
    try:
        raw       = Path("/proc/uptime").read_text().split()[0]
        total_sec = int(float(raw))
    except (OSError, ValueError, IndexError) as exc:
        log.error("Failed to read uptime: %s", exc)
        return "unknown"

    days    = total_sec // 86400
    hours   = (total_sec % 86400) // 3600
    minutes = (total_sec % 3600)  // 60

    if days > 0:
        return f"{days}d {hours}h {minutes}m"
    if hours > 0:
        return f"{hours}h {minutes}m"
    return f"{minutes}m"
