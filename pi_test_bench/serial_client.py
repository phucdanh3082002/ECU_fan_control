from __future__ import annotations

import time
from dataclasses import dataclass

import serial


STATUS_STATES = ("NORMAL", "SAFE_MODE", "FAULT_MODE")
ERROR_REASONS = ("UNKNOWN_OR_INVALID_COMMAND", "EMPTY_COMMAND", "COMMAND_TOO_LONG")


@dataclass
class ProtocolResponse:
    line: str
    fields: dict[str, str]

    @property
    def response_type(self) -> str:
        return self.fields.get("TYPE", "")


class SerialClient:
    def __init__(
        self,
        port: str,
        baud_rate: int = 115200,
        read_timeout_s: float = 0.25,
        startup_delay_s: float = 2.0,
    ) -> None:
        self.port = port
        self.baud_rate = baud_rate
        self.read_timeout_s = read_timeout_s
        self.startup_delay_s = startup_delay_s
        self._serial: serial.Serial | None = None

    def open(self) -> None:
        self._serial = serial.Serial(
            port=self.port,
            baudrate=self.baud_rate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=self.read_timeout_s,
            write_timeout=1.0,
            rtscts=False,
            dsrdtr=False,
        )
        self._serial.dtr = False
        self._serial.rts = False
        time.sleep(self.startup_delay_s)
        self.drain_input()

    def close(self) -> None:
        if self._serial is not None and self._serial.is_open:
            self._serial.close()

    def drain_input(self) -> None:
        self._require_open().reset_input_buffer()

    def send_command(self, command: str, response_timeout_s: float = 5.0) -> ProtocolResponse:
        port = self._require_open()
        port.reset_input_buffer()
        port.write(f"{command}\r\n".encode("ascii"))
        port.flush()
        return self.read_protocol_response(response_timeout_s)

    def read_protocol_response(self, timeout_s: float = 5.0) -> ProtocolResponse:
        deadline = time.monotonic() + timeout_s

        while time.monotonic() < deadline:
            raw_line = self._require_open().readline()
            if not raw_line:
                continue

            line = extract_protocol_line(raw_line.decode("utf-8", errors="replace").strip())
            if line.startswith("STATUS,") or line.startswith("ERROR,"):
                return ProtocolResponse(line=line, fields=parse_protocol_line(line))

        raise TimeoutError(f"No protocol response received within {timeout_s:.1f}s")

    def _require_open(self) -> serial.Serial:
        if self._serial is None or not self._serial.is_open:
            raise RuntimeError("Serial port is not open")
        return self._serial

    def __enter__(self) -> "SerialClient":
        self.open()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:  # type: ignore[no-untyped-def]
        self.close()


def extract_protocol_line(line: str) -> str:
    if line.startswith("STATUS,"):
        for state in STATUS_STATES:
            marker = f"STATE={state}"
            marker_index = line.find(marker)
            if marker_index >= 0:
                return line[: marker_index + len(marker)]

    if line.startswith("ERROR,"):
        for reason in ERROR_REASONS:
            marker = f"REASON={reason}"
            marker_index = line.find(marker)
            if marker_index >= 0:
                return line[: marker_index + len(marker)]

    return line


def parse_protocol_line(line: str) -> dict[str, str]:
    parts = line.split(",")
    fields: dict[str, str] = {"TYPE": parts[0]}

    for part in parts[1:]:
        if "=" not in part:
            continue
        key, value = part.split("=", 1)
        fields[key] = value

    return fields
