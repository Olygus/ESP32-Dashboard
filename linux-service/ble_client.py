"""
ble_client.py — BLE central using bleak / BlueZ.

Maintains a persistent connection to the ESP32.  On each connect,
fires on_connected so the caller can send an immediate update.
Automatically rescans and reconnects after any disconnect.
"""
from __future__ import annotations

import asyncio
import logging
from typing import Awaitable, Callable, Optional

from bleak import BleakClient, BleakScanner

log = logging.getLogger("ble_client")


class BLEClient:
    def __init__(
        self,
        device_name:         str,
        service_uuid:        str,
        characteristic_uuid: str,
        scan_timeout:        float = 15.0,
        reconnect_delay:     float = 10.0,
    ) -> None:
        self._device_name         = device_name
        self._service_uuid        = service_uuid
        self._characteristic_uuid = characteristic_uuid
        self._scan_timeout        = scan_timeout
        self._reconnect_delay     = reconnect_delay

        self._client:    Optional[BleakClient] = None
        self._connected: bool = False
        self._stop:      bool = False

    # ------------------------------------------------------------------ #

    def is_connected(self) -> bool:
        return (
            self._connected
            and self._client is not None
            and self._client.is_connected
        )

    def stop(self) -> None:
        """Signal the connect loop to exit after the current sleep."""
        self._stop = True

    async def send(self, payload: str) -> bool:
        """Write payload to the BLE characteristic.  Returns False on error."""
        if not self.is_connected() or self._client is None:
            return False
        try:
            data = payload.encode("utf-8")
            await self._client.write_gatt_char(
                self._characteristic_uuid,
                data,
                response=False,
            )
            return True
        except Exception as exc:
            log.error("Send failed: %s", exc)
            self._connected = False
            return False

    # ------------------------------------------------------------------ #

    async def connect_loop(
        self,
        on_connected:    Callable[[], Awaitable[None]],
        on_disconnected: Callable[[], Awaitable[None]],
    ) -> None:
        """
        Scan → connect → notify → keep-alive → disconnect → repeat.
        Runs until stop() is called.
        """
        while not self._stop:
            device = None
            try:
                log.info("Scanning for '%s' (timeout: %gs)…",
                         self._device_name, self._scan_timeout)
                device = await BleakScanner.find_device_by_name(
                    self._device_name,
                    timeout=self._scan_timeout,
                )
            except Exception as exc:
                log.error("Scan error: %s", exc)

            if device is None:
                log.warning("'%s' not found — retrying in %gs.",
                            self._device_name, self._reconnect_delay)
                await asyncio.sleep(self._reconnect_delay)
                continue

            try:
                async with BleakClient(device) as client:
                    self._client    = client
                    self._connected = True
                    log.info("Connected to %s (%s).",
                             self._device_name, device.address)

                    await on_connected()

                    # Keep the connection alive until it drops or stop() fires.
                    while client.is_connected and not self._stop:
                        await asyncio.sleep(1.0)

                    self._connected = False
                    self._client    = None

                    if not self._stop:
                        await on_disconnected()
                        log.info("Disconnected — reconnecting in %gs.",
                                 self._reconnect_delay)
                        await asyncio.sleep(self._reconnect_delay)

            except Exception as exc:
                log.error("Connection error: %s — retrying in %gs.",
                          exc, self._reconnect_delay)
                self._connected = False
                self._client    = None
                if not self._stop:
                    await asyncio.sleep(self._reconnect_delay)
