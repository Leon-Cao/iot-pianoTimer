"""Protocol regression tests: no serial device required."""
import importlib.util
import io
from pathlib import Path
import unittest
import wave
import zlib

spec = importlib.util.spec_from_file_location("export_recording", Path(__file__).resolve().parents[1] / "tools/export_recording.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

class RecordingTest(unittest.TestCase):
    def packet(self):
        stream = io.BytesIO()
        with wave.open(stream, "wb") as w:
            w.setnchannels(1); w.setsampwidth(2); w.setframerate(16000); w.writeframes(bytes(160000))
        data = stream.getvalue()
        return data, b"boot log\nWAV_BEGIN 160044\r\n" + data + f"\nWAV_END {zlib.crc32(data):08x}\r\n".encode()
    def test_success(self):
        data, packet = self.packet()
        self.assertEqual(module.receive_recording(io.BytesIO(packet)), data)
    def test_corrupt_audio(self):
        _, packet = self.packet(); packet = bytearray(packet); packet[200] ^= 1
        with self.assertRaisesRegex(ValueError, "checksum"):
            module.receive_recording(io.BytesIO(packet))
    def test_incomplete(self):
        _, packet = self.packet()
        with self.assertRaisesRegex(ValueError, "interrupted"):
            module.receive_recording(io.BytesIO(packet[:100]))
    def test_missing(self):
        with self.assertRaisesRegex(ValueError, "missing"):
            module.receive_recording(io.BytesIO(b"RECORD_ERROR missing or incomplete file\n"))
    def test_wrong_size(self):
        with self.assertRaisesRegex(ValueError, "size"):
            module.receive_recording(io.BytesIO(b"WAV_BEGIN 999999999\n"))

if __name__ == '__main__': unittest.main()
