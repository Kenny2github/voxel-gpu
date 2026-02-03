import struct
import sys
from typing import List, Tuple
import math
from tqdm import tqdm


def parse_obj(obj_path: str) -> Tuple[List[List[float]], List[List[int]]]:
    """Parse OBJ file and return vertices and faces."""
    vertices = []
    faces = []

    with open(obj_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue

            parts = line.split()
            if parts[0] == 'v':
                vertices.append([float(x) for x in parts[1:4]])
            elif parts[0] == 'f':
                face = []
                for part in parts[1:]:
                    vertex_idx = int(part.split('/')[0]) - 1
                    face.append(vertex_idx)
                faces.append(face)

    return vertices, faces

def get_bounds(vertices: List[List[float]]) -> Tuple[float, float, float, float, float, float]:
    """Get min and max coordinates of vertices."""
    min_x = min_y = min_z = float('inf')
    max_x = max_y = max_z = float('-inf')

    for v in vertices:
        min_x = min(min_x, v[0])
        max_x = max(max_x, v[0])
        min_y = min(min_y, v[1])
        max_y = max(max_y, v[1])
        min_z = min(min_z, v[2])
        max_z = max(max_z, v[2])

    return min_x, max_x, min_y, max_y, min_z, max_z

def triangle_aabb_intersect(v0: List[float], v1: List[float], v2: List[float],
                            box_min: List[float], box_max: List[float]) -> bool:
    """Check if a triangle intersects with an axis-aligned bounding box using SAT."""
    # Triangle edges
    edges = [
        [v1[i] - v0[i] for i in range(3)],
        [v2[i] - v1[i] for i in range(3)],
        [v0[i] - v2[i] for i in range(3)]
    ]

    # Box normals (axes)
    axes = [[1, 0, 0], [0, 1, 0], [0, 0, 1]]

    # Triangle normal
    edge1 = [v1[i] - v0[i] for i in range(3)]
    edge2 = [v2[i] - v0[i] for i in range(3)]
    normal = [
        edge1[1] * edge2[2] - edge1[2] * edge2[1],
        edge1[2] * edge2[0] - edge1[0] * edge2[2],
        edge1[0] * edge2[1] - edge1[1] * edge2[0]
    ]
    axes.append(normal)

    # Cross products of edges and box axes
    for edge in edges:
        for axis in [[1, 0, 0], [0, 1, 0], [0, 0, 1]]:
            cross = [edge[1] * axis[2] - edge[2] * axis[1],
                     edge[2] * axis[0] - edge[0] * axis[2],
                     edge[0] * axis[1] - edge[1] * axis[0]]
            axes.append(cross)

    # Check all axes
    for axis in axes:
        length = math.sqrt(sum(x*x for x in axis))
        if length < 1e-6:
            continue
        axis = [x / length for x in axis]

        # Project triangle
        t_min = min(sum(v[i] * axis[i] for i in range(3)) for v in [v0, v1, v2])
        t_max = max(sum(v[i] * axis[i] for i in range(3)) for v in [v0, v1, v2])

        # Project box
        b_min = sum(box_min[i] * axis[i] if axis[i] > 0 else box_max[i] * axis[i] for i in range(3))
        b_max = sum(box_max[i] * axis[i] if axis[i] > 0 else box_min[i] * axis[i] for i in range(3))

        if t_max < b_min or b_max < t_min:
            return False

    return True

def convert_obj_to_voxel(N: int, WN: int, obj_path: str, ox: int, oy: int, oz: int, output_path: str = None) -> bytearray:
    """Convert OBJ file to voxel data.

    Args:
        N: Size of voxel grid (N x N x N)
        WN: World space size
        obj_path: Path to OBJ file
        ox, oy, oz: Object center position in world space
        output_path: Optional output file path
    """
    vertices, faces = parse_obj(obj_path)

    if not vertices or not faces:
        raise ValueError("Invalid OBJ file")

    min_x, max_x, min_y, max_y, min_z, max_z = get_bounds(vertices)

    # Calculate voxel size based on object bounds
    voxel_size_x = (max_x - min_x) / N if max_x > min_x else 1
    voxel_size_y = (max_y - min_y) / N if max_y > min_y else 1
    voxel_size_z = (max_z - min_z) / N if max_z > min_z else 1

    voxel_size = max(voxel_size_x, voxel_size_y, voxel_size_z)

    # Initialize voxel grid
    voxel_data = bytearray(WN * WN * WN)

    bottom_x = ox - N//2
    bottom_y = oy - N//2
    bottom_z = oz - N//2

    # Check each face and mark intersected voxels
    print("Total faces: ", len(faces))
    for face in tqdm(faces, desc="Processing faces"):
        if len(face) < 3:
            continue

        v0 = vertices[face[0]]
        v1 = vertices[face[1]]
        v2 = vertices[face[2]]

        # Calculate triangle bounding box
        tri_min_x = min(v0[0], v1[0], v2[0])
        tri_max_x = max(v0[0], v1[0], v2[0])
        tri_min_y = min(v0[1], v1[1], v2[1])
        tri_max_y = max(v0[1], v1[1], v2[1])
        tri_min_z = min(v0[2], v1[2], v2[2])
        tri_max_z = max(v0[2], v1[2], v2[2])

        # Convert triangle bounds to voxel coordinates
        voxel_min_x = max(0, int((tri_min_x - min_x) / voxel_size))
        voxel_max_x = min(N - 1, int((tri_max_x - min_x) / voxel_size))
        voxel_min_y = max(0, int((tri_min_y - min_y) / voxel_size))
        voxel_max_y = min(N - 1, int((tri_max_y - min_y) / voxel_size))
        voxel_min_z = max(0, int((tri_min_z - min_z) / voxel_size))
        voxel_max_z = min(N - 1, int((tri_max_z - min_z) / voxel_size))

        # Only check voxels within triangle's bounding box
        for y in range(voxel_min_y, voxel_max_y + 1):
            for z in range(voxel_min_z, voxel_max_z + 1):
                for x in range(voxel_min_x, voxel_max_x + 1):
                    voxel_min = [
                        min_x + x * voxel_size,
                        min_y + y * voxel_size,
                        min_z + z * voxel_size
                    ]
                    voxel_max = [
                        voxel_min[0] + voxel_size,
                        voxel_min[1] + voxel_size,
                        voxel_min[2] + voxel_size
                    ]

                    if triangle_aabb_intersect(v0, v1, v2, voxel_min, voxel_max):
                        addr = (bottom_x + x) + (bottom_z + z) * WN + (bottom_y + y) * WN * WN
                        voxel_data[addr] = 0x1

    # Write to file if output path provided
    if output_path:
        if output_path.endswith(".h"):
            with open(output_path, 'w') as f:
                f.write(f'''\
#ifndef {output_path[:-2].upper()}_H
#define {output_path[:-2].upper()}_H

#include "firmware/firmware.h"

void load_{output_path[:-2]}() {{
''')
                for x in range(WN):
                    for z in range(WN):
                        for y in range(WN):
                            if voxel_data[y * WN * WN + z * WN + x]:
                                f.write(f'''\
    set_voxel((v_pos){{ .x = {x}, .y = {y}, .z = {z}}}, 1);
''')
                f.write('}')
                f.write("\n\n#endif")
        else:
            with open(output_path, 'wb') as f:
                f.write(voxel_data)


    return voxel_data

if __name__ == "__main__":
    if len(sys.argv) < 7:
        print("Usage: python convert-obj.py <N> <WN> <obj_path> <ox> <oy> <oz> [output_path]")
        sys.exit(1)

    N = int(sys.argv[1])
    WN = int(sys.argv[2])
    obj_path = sys.argv[3]
    ox = int(sys.argv[4])
    oy = int(sys.argv[5])
    oz = int(sys.argv[6])
    output_path = sys.argv[7] if len(sys.argv) > 7 else "voxel_data.bin"

    voxel_data = convert_obj_to_voxel(N, WN, obj_path, ox, oy, oz, output_path)
    print(f"Converted OBJ to voxel grid ({N}x{N}x{N})")
    print(f"World space size: {WN}")
    print(f"Object center position: ({ox}, {oy}, {oz})")
    print(f"Output written to: {output_path}")
