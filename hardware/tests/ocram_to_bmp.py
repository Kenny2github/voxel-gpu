#!/usr/bin/python3
from PIL import Image

WIDTH = 320
HEIGHT = 240

def hex_to_bytes(filename: str) -> bytearray:
    data = bytearray()
    with open(filename) as f:
        for i, line in enumerate(f):
            if line.startswith('//'):
                continue
            word = int(line, 16)
            data.extend(word.to_bytes(4, 'little'))
    return data

def vga_to_rgb(data: bytearray) -> bytearray:
    pixels = bytearray()
    for row in range(HEIGHT):
        for col in range(WIDTH):
            coord = (row << 10) | (col << 1)
            word = int.from_bytes(data[coord:coord+2], 'little')
            r = ((word & 0xf100) >> (6 + 5)) * 0xff // 0x1f
            g = ((word & 0x07e0) >> 5) * 0xff // 0x3f
            b = (word & 0x001f) * 0xff // 0x1f
            pixels.extend((r, g, b))
    return pixels

if __name__ == '__main__':
    Image.frombytes(
        'RGB', (WIDTH, HEIGHT),
        vga_to_rgb(hex_to_bytes('ocram.hex'))
    ).save('ocram.bmp')
