#!/usr/bin/env python3
"""Optional Pi Zero 2 W UART companion. No shell/HDMI access in v1.

Run on Raspberry Pi OS: python3 calctus_agent.py /dev/serial0
For protocol tests: python3 calctus_agent.py --stdio
Uses only Python 3 standard library; serial must be configured as 115200 8N1.
"""

import os
import select
import struct
import sys
import termios

MAX_PAYLOAD = 48


def crc(data):
    value = 0xffff
    for byte in data:
        value ^= byte << 8
        for _ in range(8):
            value = ((value << 1) ^ (0x1021 if value & 0x8000 else 0)) & 0xffff
    return value


def frame(sequence, command, payload=b""):
    if len(payload) > MAX_PAYLOAD:
        raise ValueError("payload too long")
    body = bytes((1, sequence, command, len(payload))) + payload
    return b"\xa5\x5a" + body + struct.pack("<H", crc(body))


def reply(command, payload):
    if command == 1 and not payload:
        return b"OK"
    if command == 2 and not payload:
        return b"Pi Zero 2 W (agent v1)"
    if command == 3 and len(payload) == 4:
        number = struct.unpack("<i", payload)[0]
        if -46340 <= number <= 46340:
            return struct.pack("<i", number * number)
    return None


def serve(read_fd, write_fd):
    buffer = bytearray()
    while True:
        readable, _, _ = select.select([read_fd], [], [], 1)
        if not readable:
            continue
        chunk = os.read(read_fd, 128)
        if not chunk:
            return
        buffer.extend(chunk)
        while len(buffer) >= 8:
            if buffer[:2] != b"\xa5\x5a":
                del buffer[0]
                continue
            if buffer[2] != 1 or buffer[5] > MAX_PAYLOAD:
                del buffer[0]
                continue
            length = buffer[5] + 8
            if len(buffer) < length:
                break
            data = bytes(buffer[:length])
            del buffer[:length]
            if crc(data[2:-2]) != struct.unpack("<H", data[-2:])[0]:
                continue
            payload = reply(data[4], data[6:-2])
            if payload is not None:
                answer = frame(data[3], data[4], payload)
                while answer:
                    answer = answer[os.write(write_fd, answer):]


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: calctus_agent.py /dev/serial0 | --stdio")
    if sys.argv[1] == "--stdio":
        serve(sys.stdin.fileno(), sys.stdout.fileno())
        return
    fd = os.open(sys.argv[1], os.O_RDWR | os.O_NOCTTY)
    try:
        settings = termios.tcgetattr(fd)
        settings[0] = settings[1] = settings[3] = 0
        settings[2] &= ~(termios.CSIZE | termios.PARENB | termios.CSTOPB)
        settings[2] |= termios.CS8 | termios.CREAD | termios.CLOCAL
        settings[4] = settings[5] = termios.B115200
        settings[6][termios.VMIN] = 1
        settings[6][termios.VTIME] = 0
        termios.tcsetattr(fd, termios.TCSANOW, settings)
        serve(fd, fd)
    finally:
        os.close(fd)


if __name__ == "__main__":
    main()
