"""
stylus.py — Wacom stylus battery via external shell script.

The script at stylus.script_path is executed and its stdout is parsed
by the active StylusParser.  If the script is missing, times out, or
returns nothing parseable, collect() returns None (→ JSON null →
ESP32 displays "Stylus: N/A").

To support a custom script output format, subclass StylusParser:

    class MyParser(StylusParser):
        def parse(self, output: str) -> Optional[int]:
            # return 0-100 or None
            ...

    sty = stylus.collect(path, parser=MyParser())
"""
from __future__ import annotations

import logging
import re
import subprocess
from abc import ABC, abstractmethod
from pathlib import Path
from typing import Optional

log = logging.getLogger("collectors.stylus")


# ------------------------------------------------------------------ #
# Parser interface + default implementation
# ------------------------------------------------------------------ #

class StylusParser(ABC):
    @abstractmethod
    def parse(self, output: str) -> Optional[int]:
        """Parse script stdout; return an integer 0-100 or None."""
        ...


class RegexParser(StylusParser):
    """
    Default parser: finds the first bare integer in the range 0-100.
    Works for scripts that output plain text like:
        Battery level: 87%
        87
    """
    _PATTERN = re.compile(r"\b([0-9]{1,3})\b")

    def parse(self, output: str) -> Optional[int]:
        for m in self._PATTERN.finditer(output):
            v = int(m.group(1))
            if 0 <= v <= 100:
                return v
        return None


# ------------------------------------------------------------------ #
# Main entry point
# ------------------------------------------------------------------ #

def collect(
    script_path: str,
    parser: Optional[StylusParser] = None,
) -> Optional[int]:
    """
    Run the stylus battery script and return 0-100, or None on failure.
    """
    if parser is None:
        parser = RegexParser()

    path = Path(script_path)
    if not path.exists():
        log.debug("Stylus script not found: %s — reporting N/A.", script_path)
        return None

    try:
        result = subprocess.run(
            [str(path)],
            capture_output=True,
            text=True,
            timeout=5,
        )
    except subprocess.TimeoutExpired:
        log.warning("Stylus script timed out.")
        return None
    except Exception as exc:
        log.error("Stylus script error: %s", exc)
        return None

    if result.returncode != 0:
        log.warning("Stylus script exited %d: %s",
                    result.returncode, result.stderr.strip())
        return None

    parsed = parser.parse(result.stdout)
    if parsed is None:
        log.debug("Stylus parser found no value in: %r", result.stdout)
    return parsed
