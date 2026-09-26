#!/usr/bin/env python3
"""Stream frames to the lamp over UDP (protocol: docs/lamp/protocol.md, section 3).

Examples:
    python tools/stream_frames.py --host indicator.local plasma
    python tools/stream_frames.py --host 192.168.4.1 image picture.png
    python tools/stream_frames.py --host indicator.local speckles --fps 30

Needs numpy; the `image` pattern also needs Pillow (pip install numpy pillow).
"""

import argparse
import math
import socket
import struct
import time

import numpy as np

W, H = 128, 64
PORT = 7777
ROWS_PER_PACKET = 5  # RGB565: 5 * 128 * 2 + 8 = 1288 bytes < MTU


def to_rgb565(img: np.ndarray) -> np.ndarray:
    """img: HxWx3 uint8 -> HxW uint16 little endian RGB565."""
    r = (img[..., 0].astype(np.uint16) >> 3) << 11
    g = (img[..., 1].astype(np.uint16) >> 2) << 5
    b = img[..., 2].astype(np.uint16) >> 3
    return (r | g | b).astype("<u2")


def send_face(sock, addr, img: np.ndarray, face: int) -> None:
    """face: 0 front, 1 back, 2 both."""
    data = to_rgb565(img)
    for y0 in range(0, H, ROWS_PER_PACKET):
        rows = min(ROWS_PER_PACKET, H - y0)
        last = 1 if y0 + rows >= H else 0
        header = struct.pack("<2sBBBBBB", b"IF", 1, 0, face, y0, rows, last)
        sock.sendto(header + data[y0 : y0 + rows].tobytes(), addr)


# ---------------------------------------------------------------- patterns

def plasma(t: float) -> np.ndarray:
    y, x = np.mgrid[0:H, 0:W].astype(np.float32)
    v = (np.sin(x / 11 + t) + np.sin(y / 7 - t * 1.3) + np.sin((x + y) / 17 + t * 0.7)
         + np.sin(np.hypot(x - W / 2, y - H / 2) / 8 - t * 2))
    h = (v / 8 + 0.5 + t * 0.05) % 1.0
    rgb = np.stack([np.sin(2 * math.pi * (h + k / 3)) * 0.5 + 0.5 for k in range(3)], axis=-1)
    return (rgb * 255).astype(np.uint8)


class Speckles:
    """PC-side 'violet in speckles', handy to compare with the firmware scene."""

    def __init__(self, n=60, seed=1):
        rng = np.random.default_rng(seed)
        self.pts = rng.integers([0, 0], [W, H], size=(n, 2))
        self.phase = rng.random(n) * 2 * math.pi

    def __call__(self, t: float) -> np.ndarray:
        img = np.zeros((H, W, 3), np.uint8)
        img[:] = (109, 0, 204)
        for (x, y), ph in zip(self.pts, self.phase):
            k = 0.65 + 0.35 * math.sin(t * 4 + ph)
            c = np.array([109, 0, 204]) * (1 - k) + np.array([0, 255, 0]) * k
            img[max(0, y - 1) : y + 2, max(0, x - 1) : x + 2] = c.astype(np.uint8)
        return img


def load_image(path: str):
    from PIL import Image  # optional dependency

    img = Image.open(path).convert("RGB").resize((W, H), Image.LANCZOS)
    arr = np.asarray(img, dtype=np.uint8)
    return lambda t: arr


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--host", default="indicator.local")
    ap.add_argument("--fps", type=float, default=25.0)
    ap.add_argument("--face", type=int, default=2, choices=[0, 1, 2], help="0 front, 1 back, 2 both")
    ap.add_argument("--seconds", type=float, default=0, help="0 = until Ctrl+C")
    ap.add_argument("pattern", choices=["plasma", "speckles", "image"])
    ap.add_argument("path", nargs="?", help="image file for the 'image' pattern")
    args = ap.parse_args()

    if args.pattern == "plasma":
        gen = plasma
    elif args.pattern == "speckles":
        gen = Speckles()
    else:
        if not args.path:
            ap.error("image pattern needs a path")
        gen = load_image(args.path)

    addr = (socket.gethostbyname(args.host), PORT)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print(f"streaming {args.pattern} to {addr[0]}:{PORT} at {args.fps} fps, Ctrl+C to stop")

    period = 1.0 / args.fps
    start = time.monotonic()
    frames = 0
    try:
        while True:
            t = time.monotonic() - start
            if args.seconds and t > args.seconds:
                break
            send_face(sock, addr, gen(t), args.face)
            frames += 1
            time.sleep(max(0.0, start + frames * period - time.monotonic()))
    except KeyboardInterrupt:
        pass
    print(f"sent {frames} frames")


if __name__ == "__main__":
    main()
