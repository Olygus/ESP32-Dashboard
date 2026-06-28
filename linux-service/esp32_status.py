#!/usr/bin/env python3
"""
ESP32 Status Display — Linux systemd user service.

Collects system metrics every N minutes (default: 45) and transmits
them to the ESP32 via BLE.  Also sends an immediate update on each
fresh connect (covers suspend/resume — the connection drops during
suspend and the reconnect triggers a fresh packet on resume).

Usage
-----
  Direct:   python3 esp32_status.py [--config /path/to/config.toml]
  Service:  systemctl --user start esp32-status

Environment
-----------
  ESP32_STATUS_CONFIG   Override config file path.
"""
from __future__ import annotations

import argparse
import asyncio
import logging
import os
import signal
import sys
from pathlib import Path

# ---- tomllib (Python 3.11+) or tomli (backport) ----
try:
    import tomllib
except ImportError:
    try:
        import tomli as tomllib  # type: ignore[no-redef]
    except ImportError:
        sys.exit(
            "Error: tomllib not available.\n"
            "Either upgrade to Python 3.11+, or install tomli:\n"
            "  pip install tomli"
        )

from collectors import battery, uptime, time_collector, stylus
from collectors.stylus import RegexParser
from serializer import StatusData
from ble_client import BLEClient

# ================================================================ #
# Logging
# ================================================================ #

def _setup_logging(level_str: str) -> None:
    level = getattr(logging, level_str.upper(), logging.INFO)
    logging.basicConfig(
        level=level,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
        stream=sys.stdout,
    )


log = logging.getLogger("esp32_status")

# ================================================================ #
# Config
# ================================================================ #

_DEFAULT_CONFIG = Path(__file__).with_name("config.toml")


def _load_config(path: Path) -> dict:
    with open(path, "rb") as fh:
        return tomllib.load(fh)

# ================================================================ #
# Status collection
# ================================================================ #

def _collect(cfg: dict) -> StatusData:
    """Gather all data sources and return a populated StatusData."""
    t   = time_collector.collect()
    bat = battery.collect()
    up  = uptime.collect()
    sty = stylus.collect(
        cfg["stylus"]["script_path"],
        parser=RegexParser(),
    )

    return StatusData(
        date=t["date"],
        time=t["time"],
        uptime=up,
        batteryPercent=bat.percent,
        batteryRemaining=bat.remaining,
        stylusPercent=sty,       # None is fine; serializer omits it
    )

# ================================================================ #
# Service
# ================================================================ #

class StatusService:
    """
    Async service that:
      1. Keeps a persistent BLE connection via BLEClient.connect_loop().
      2. Sends an immediate update on each connect event.
      3. Sends a scheduled update every interval_seconds.
    """

    def __init__(self, cfg: dict) -> None:
        self._cfg      = cfg
        self._interval = cfg["update"]["interval_seconds"]
        self._stop_evt = asyncio.Event()

        self._ble = BLEClient(
            device_name=cfg["ble"]["device_name"],
            service_uuid=cfg["ble"]["service_uuid"],
            characteristic_uuid=cfg["ble"]["characteristic_uuid"],
            scan_timeout=float(cfg["ble"].get("scan_timeout_seconds", 15)),
            reconnect_delay=float(cfg["ble"].get("reconnect_delay_seconds", 10)),
        )

    async def run(self) -> None:
        log.info("Service starting.  Update interval: %ds.", self._interval)

        connect_task = asyncio.create_task(
            self._ble.connect_loop(
                on_connected=self._on_connected,
                on_disconnected=self._on_disconnected,
            )
        )
        update_task = asyncio.create_task(self._periodic_loop())

        await self._stop_evt.wait()

        log.info("Stopping service…")
        self._ble.stop()
        connect_task.cancel()
        update_task.cancel()
        await asyncio.gather(connect_task, update_task, return_exceptions=True)
        log.info("Service stopped.")

    def request_stop(self) -> None:
        self._stop_evt.set()

    # ---- BLE event handlers ----

    async def _on_connected(self) -> None:
        log.info("Connected to ESP32 — sending immediate update.")
        await self._send()

    async def _on_disconnected(self) -> None:
        log.info("Disconnected from ESP32.")

    # ---- Periodic update loop ----

    async def _periodic_loop(self) -> None:
        while True:
            await asyncio.sleep(self._interval)
            if self._ble.is_connected():
                log.info("Scheduled update.")
                await self._send()
            else:
                log.debug("Not connected — skipping scheduled update.")

    # ---- Transmit ----

    async def _send(self) -> None:
        try:
            data    = _collect(self._cfg)
            payload = data.to_json()
            log.info("TX: %s", payload)
            success = await self._ble.send(payload)
            if not success:
                log.warning("Send returned False — will retry on next cycle.")
        except Exception as exc:  # noqa: BLE001
            log.error("Failed to collect/send status: %s", exc)


# ================================================================ #
# Entry point
# ================================================================ #

def main() -> None:
    parser = argparse.ArgumentParser(description="ESP32 Status BLE service")
    parser.add_argument(
        "--config",
        type=Path,
        default=Path(os.environ.get("ESP32_STATUS_CONFIG", _DEFAULT_CONFIG)),
        help="Path to config.toml",
    )
    args = parser.parse_args()

    if not args.config.exists():
        sys.exit(f"Config file not found: {args.config}")

    cfg = _load_config(args.config)
    _setup_logging(cfg.get("logging", {}).get("level", "INFO"))

    service = StatusService(cfg)

    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)

    def _handle_signal(sig: int, _frame: object) -> None:
        log.info("Received signal %d — shutting down.", sig)
        loop.call_soon_threadsafe(service.request_stop)

    signal.signal(signal.SIGTERM, _handle_signal)
    signal.signal(signal.SIGINT,  _handle_signal)

    try:
        loop.run_until_complete(service.run())
    finally:
        loop.close()


if __name__ == "__main__":
    main()
