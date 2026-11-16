#!/usr/bin/env python3
"""Real Pi agent, framed request/response including CRC rejection."""
import pathlib
import select
import struct
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "platform/pi-zero-2"))
import calctus_agent as agent


def exchange(process, command, payload=b"", seq=7):
    process.stdin.write(agent.frame(seq, command, payload))
    process.stdin.flush()
    ready, _, _ = select.select([process.stdout], [], [], 2)
    assert ready, "agent response timed out"
    header = process.stdout.read(6)
    assert header[:5] == bytes((0xa5, 0x5a, 1, seq, command))
    suffix = process.stdout.read(header[5] + 2)
    assert agent.crc(header[2:] + suffix[:-2]) == struct.unpack("<H", suffix[-2:])[0]
    return suffix[:-2]


with subprocess.Popen([sys.executable, agent.__file__, "--stdio"],
                      stdin=subprocess.PIPE, stdout=subprocess.PIPE) as process:
    assert exchange(process, 1) == b"OK"
    assert exchange(process, 2) == b"Pi Zero 2 W (agent v1)"
    assert struct.unpack("<i", exchange(process, 3, struct.pack("<i", 123)))[0] == 15129
    assert exchange(process, 3, struct.pack("<i", -10)) == struct.pack("<i", 100)
    bad = bytearray(agent.frame(7, 1))
    bad[-1] ^= 0x01
    process.stdin.write(bad)
    process.stdin.flush()
    assert not select.select([process.stdout], [], [], 0.1)[0], "bad CRC accepted"
    assert exchange(process, 1) == b"OK"  # Parser resynchronises.
    assert agent.reply(3, struct.pack("<i", 100000)) is None
    process.stdin.close()
    assert process.wait(timeout=2) == 0
print("Pi agent: UART frame, ping, info, remote square and CRC passed")
