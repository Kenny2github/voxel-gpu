#!/usr/bin/python3
from dataclasses import dataclass, field
import math
from typing import NamedTuple

from PIL import Image
from ocram_to_bmp import vga_to_rgb

def clog2(x: float) -> int:
    return math.ceil(math.log2(x))

Voxel = tuple[tuple[int, int, int], int]

class vec3(NamedTuple):
    x: float
    y: float
    z: float

@dataclass
class cam1:
    pos: vec3
    look: vec3

@dataclass
class cam3:
    pos: vec3
    look0: vec3
    look1: vec3
    look2: vec3
    look3: vec3

def fdiv(a: float, b: float) -> float:
    try:
        return a / b
    except ZeroDivisionError:
        if a:
            return math.copysign(math.inf, a)
        return math.nan
    

@dataclass
class pixel_shader:
    cam: cam1

    closest_t = float('inf')
    closest_voxel = 0
    pixel = 0

    def rasterize(self, voxel: Voxel) -> None:
        (voxel_x, voxel_y, voxel_z), voxel_id = voxel
        lx, ly, lz = voxel_x, voxel_y, voxel_z
        hx, hy, hz = voxel_x + 1, voxel_y + 1, voxel_z + 1

        tlx, tly, tlz, thx, thy, thz = 0, 0, 0, 0, 0, 0

        tlx = fdiv((lx-self.cam.pos.x), self.cam.look.x)
        tly = fdiv((ly-self.cam.pos.y), self.cam.look.y)
        tlz = fdiv((lz-self.cam.pos.z), self.cam.look.z)
        
        thx = fdiv((hx-self.cam.pos.x), self.cam.look.x)
        thy = fdiv((hy-self.cam.pos.y), self.cam.look.y)
        thz = fdiv((hz-self.cam.pos.z), self.cam.look.z)

        min_A_B_x = min(tlx, thx)
        min_A_B_y = min(tly, thy)
        min_A_B_z = min(tlz, thz)

        max_A_B_x = max(tlx, thx)
        max_A_B_y = max(tly, thy)
        max_A_B_z = max(tlz, thz)
        
        t_min = max(min_A_B_x, min_A_B_y, min_A_B_z)
        t_max = min(max_A_B_x, max_A_B_y, max_A_B_z)

        t = t_min if t_min > 0 else t_max
        if t > 0 and t_min <= t_max and t < self.closest_t:
            self.closest_t = t
            self.closest_voxel = voxel_id

    def shade(self, shade_entry: tuple[int, int]) -> None:
        palette_entry, voxel_id = shade_entry
        if voxel_id == self.closest_voxel:
            self.pixel = palette_entry

def lerp2(p0: float, p1: float, p2: float, p3: float,
          x: float, y: float, X: float, Y: float) -> float:
    lerp_x = (p1 - p0) * x / X
    lerp_y = (p2 - p0) * y / Y
    lerp_xy = (p0 - p1 + p3 - p2) * x * y / (X * Y)
    return p0 + lerp_x + lerp_y + lerp_xy

@dataclass
class voxel_gpu:
    cam: cam3
    NUM_SHADERS: int = 200
    H_RESOLUTION: int = 320
    V_RESOLUTION: int = 240

    shaders: list[pixel_shader] = field(init=False, default_factory=list)
    mem: bytearray = field(init=False)
    start_pixel: int = field(init=False, default=0)

    def __post_init__(self) -> None:
        self.mem = bytearray(1 << (clog2(self.H_RESOLUTION) + clog2(self.V_RESOLUTION) + 1))

    def coordinate(self, start_pixel: int) -> None:
        self.start_pixel = start_pixel
        self.shaders.clear()
        for i in range(self.NUM_SHADERS):
            div_i_val = (i + start_pixel) / self.H_RESOLUTION
            shader_row = int(div_i_val)
            shader_col = (i + start_pixel) - (shader_row * self.H_RESOLUTION)
            cam_look_x = lerp2_x_val = lerp2(self.cam.look0.x, self.cam.look1.x, self.cam.look2.x, self.cam.look3.x, shader_col, shader_row, self.H_RESOLUTION-1, self.V_RESOLUTION-1)
            cam_look_y = lerp2_y_val = lerp2(self.cam.look0.y, self.cam.look1.y, self.cam.look2.y, self.cam.look3.y, shader_col, shader_row, self.H_RESOLUTION-1, self.V_RESOLUTION-1)
            cam_look_z = lerp2_z_val = lerp2(self.cam.look0.z, self.cam.look1.z, self.cam.look2.z, self.cam.look3.z, shader_col, shader_row, self.H_RESOLUTION-1, self.V_RESOLUTION-1)

            self.shaders.append(pixel_shader(cam1(self.cam.pos, vec3(cam_look_x, cam_look_y, cam_look_z))))

    def rasterize_voxel(self, voxel: Voxel) -> None:
        for shader in self.shaders:
            shader.rasterize(voxel)

    def shade_entry(self, shade_entry: tuple[int, int]) -> None:
        for shader in self.shaders:
            shader.shade(shade_entry)

    def write_pixel(self, addr: int) -> None:
        COL_BITS = clog2(self.H_RESOLUTION)
        pixel_index = (addr >> (COL_BITS + 1)) * self.H_RESOLUTION \
            + ((addr & ((1 << (COL_BITS + 1)) - 1)) >> 1) \
            - self.start_pixel
        self.mem[addr:addr+2] = self.shaders[pixel_index].pixel.to_bytes(2, 'big')

if __name__ == '__main__':
    DUT = voxel_gpu(cam3(
        vec3(4, 0, 0),
        vec3(-2.0, 2, 1.5),
        vec3(-2.0, -2, 1.5),
        vec3(-2.0, 2, -1.5),
        vec3(-2.0, -2, -1.5)
    ), NUM_SHADERS=200)

    for i in range(0, DUT.H_RESOLUTION * DUT.V_RESOLUTION, DUT.NUM_SHADERS):
        DUT.coordinate(i)
        DUT.rasterize_voxel(((-0.5, -0.5, -0.5), 1))
        DUT.rasterize_voxel(((-1, 1.5, 1.5), 1))
        DUT.rasterize_voxel(((-1, -2.5, 1.5), 1))
        DUT.rasterize_voxel(((-1, 1.5, -2.5), 1))
        DUT.rasterize_voxel(((-1, -2.5, -2.5), 1))
        DUT.shade_entry((0xFFFF, 1))
        for j in range(i, i + DUT.NUM_SHADERS):
            row, col = divmod(j, DUT.H_RESOLUTION)
            DUT.write_pixel((row << 10) | (col << 1))
    Image.frombytes(
        'RGB', (DUT.H_RESOLUTION, DUT.V_RESOLUTION),
        vga_to_rgb(DUT.mem)
    ).save('model.bmp')
