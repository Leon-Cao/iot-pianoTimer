#!/usr/bin/env python3
"""Fetch a previously saved recording; does NOT start a new recording."""
import argparse
import io
from pathlib import Path
import re
import time
import wave
import zlib


def receive_recording(port, timeout=45):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        line = port.readline()
        if line.startswith(b"RECORD_ERROR"):
            raise ValueError(line.decode(errors="replace").strip())
        if line.startswith(b"WAV_BEGIN "):
            size = int(line.split()[1])
            break
    else:
        raise ValueError("No recording header. Check firmware, recording number and port.")
    if size != 160044:
        raise ValueError("Unexpected recording size")
    data = bytearray()
    deadline = time.monotonic() + timeout
    while len(data) < size and time.monotonic() < deadline:
        chunk = port.read(min(4096, size - len(data)))
        if not chunk:
            raise ValueError("Transfer interrupted; no WAV written")
        data.extend(chunk)
    if len(data) != size:
        raise ValueError("Incomplete recording")
    marker = port.readline()  # newline following binary bytes
    if not marker.strip():
        marker = port.readline()
    match = re.fullmatch(rb"WAV_END ([0-9a-fA-F]{8})\s*", marker)
    if not match or int(match[1], 16) != zlib.crc32(data):
        raise ValueError("Recording checksum mismatch; no WAV written")
    with wave.open(io.BytesIO(data), "rb") as wav:
        if (wav.getnchannels(), wav.getsampwidth(), wav.getframerate(), wav.getnframes()) != (1, 2, 16000, 80000):
            raise ValueError("Invalid WAV audio format")
    return bytes(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--number", type=int, required=True, help="Saved number, e.g. 1 for piano-0001.wav")
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    if not 1 <= args.number <= 9999:
        parser.error("--number must be 1..9999")
    output = Path(args.output)
    if output.exists():
        parser.error("Output already exists; choose a new filename")
    import serial
    try:
        with serial.Serial(args.port, 115200, timeout=3) as port:
            time.sleep(2)
            port.reset_input_buffer()
            port.write(f"export {args.number}\n".encode())
            data = receive_recording(port)
        with output.open("xb") as file:
            file.write(data)
    except (serial.SerialException, ValueError, OSError, wave.Error) as error:
        raise SystemExit(f"Export failed: {error}\nClose Arduino Serial Monitor and check the port/recording number.") from None
    print(f"Saved {output} (5 seconds, CRC verified). Original remains on device.")


if __name__ == "__main__":
    main()
