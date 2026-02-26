#!/usr/bin/python3
from PIL import Image

WIDTH = 320
HEIGHT = 240

with open('ocram.hex') as f:
    data = bytearray()
    for i, line in enumerate(f):
        if line.startswith('//'):
            continue
        word = int(line, 16)
        data.extend(word.to_bytes(4, 'little'))

pixels = bytearray()
for row in range(HEIGHT):
    for col in range(WIDTH):
        coord = (row << 10) | (col << 1)
        word = int.from_bytes(data[coord:coord+2], 'little')
        r = ((word & 0xf100) >> (6 + 5)) * 0xff // 0x1f
        g = ((word & 0x07e0) >> 5) * 0xff // 0x3f
        b = (word & 0x001f) * 0xff // 0x1f
        pixels.extend((r, g, b))
        r = ((word & 0xf1000000) >> (6 + 5 + 16)) * 0xff // 0x1f
        g = ((word & 0x07e00000) >> (5 + 16)) * 0xff // 0x3f
        b = ((word & 0x001f0000) >> 16) * 0xff // 0x1f
        pixels.extend((r, g, b))
Image.frombytes('RGB', (256, 192), pixels).save('ocram.bmp')
