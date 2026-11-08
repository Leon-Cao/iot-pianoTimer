#!/usr/bin/env python3
"""Save MicTest 5-second mono PCM as a WAV. Close Arduino Serial Monitor first."""
import argparse, struct, time, wave
import serial
p=argparse.ArgumentParser();p.add_argument('--port',required=True);p.add_argument('--output',required=True);p.add_argument('--mode',choices=['i2s','pdm'],required=True);p.add_argument('--channel',choices=['left','right'],default='left');args=p.parse_args()
with serial.Serial(args.port,921600,timeout=1) as s:
    time.sleep(2);s.write(b'i' if args.mode=='i2s' else b'p');time.sleep(.5)
    s.write(b'l' if args.channel=='left' else b'r');time.sleep(.5);s.reset_input_buffer();s.write(b'w')
    deadline=time.monotonic()+10
    while time.monotonic()<deadline:
        line=s.readline()
        if line.startswith(b'PCM_BEGIN '):
            count=int(line.split()[1]);break
    else: raise SystemExit('No capture header: check MicTest firmware, port and mode.')
    if count!=160000: raise SystemExit('Unexpected capture length')
    data=bytearray();deadline=time.monotonic()+15
    while len(data)<count and time.monotonic()<deadline:
        data.extend(s.read(count-len(data)))
    trailer=s.read_until(b'PCM_END\r\n')
    if len(data)!=count or b'PCM_END' not in trailer: raise SystemExit('Incomplete capture; WAV not written. Check RX overflow/serial speed.')
with wave.open(args.output,'wb') as w:
    w.setnchannels(1);w.setsampwidth(2);w.setframerate(16000);w.writeframes(data)
print(f'Saved {args.output}: 5 seconds, 16 kHz mono. Listen before saving mic settings.')
