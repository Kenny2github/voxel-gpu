import bpy
import sys
import bmesh
from mathutils import Vector

def clear_scene():
    """Clear all mesh objects from the scene."""
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)

def create_voxel_cube(location: Vector, size: float):
    """Create a cube at the specified location with given size."""
    bpy.ops.mesh.primitive_cube_add(size=size, location=location)
    return bpy.context.active_object

def voxel_to_obj(N: int, voxel_path: str, voxel_size: float, output_path: str):
    """Convert voxel data to OBJ file by creating cubes for each voxel.
    
    Args:
        N: Size of voxel grid (N x N x N)
        voxel_path: Path to binary voxel data file
        voxel_size: Size of each voxel cube in model space
        output_path: Path to output OBJ file
    """
    # Clear the scene
    clear_scene()
    
    # Read voxel data
    with open(voxel_path, 'rb') as f:
        voxel_data = f.read()
    
    if len(voxel_data) != N * N * N:
        raise ValueError(f"Expected {N*N*N} bytes, got {len(voxel_data)} bytes")
    
    print(f"Processing {N}x{N}x{N} voxel grid...")
    
    # Create a list to hold all cube objects
    cube_objects = []
    voxel_count = 0
    
    # Iterate through voxel data and create cubes
    for y in range(N):
        for z in range(N):
            for x in range(N):
                addr = x + z * N + y * N * N
                
                if voxel_data[addr] == 0x1:
                    # Calculate world position (centered at origin)
                    location = Vector((
                        (x - N/2 + 0.5) * voxel_size,
                        (y - N/2 + 0.5) * voxel_size,
                        (z - N/2 + 0.5) * voxel_size
                    ))
                    
                    cube = create_voxel_cube(location, voxel_size)
                    cube_objects.append(cube)
                    voxel_count += 1
    
    print(f"Created {voxel_count} voxel cubes")
    
    # Join all cubes into a single mesh for efficiency
    if cube_objects:
        # Select all cubes
        bpy.ops.object.select_all(action='DESELECT')
        for obj in cube_objects:
            obj.select_set(True)
        bpy.context.view_layer.objects.active = cube_objects[0]
        
        # Join all selected objects
        bpy.ops.object.join()
        
        # Remove duplicate vertices and merge
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.mesh.remove_doubles(threshold=0.0001)
        bpy.ops.object.mode_set(mode='OBJECT')
        
        print("Merged voxel cubes into single mesh")
    
    # Export to OBJ (handle both old and new Blender API)
    try:
        # Try new API (Blender 3.0+)
        bpy.ops.wm.obj_export(
            filepath=output_path,
            export_selected_objects=True,
            export_materials=False,
            export_triangulated_mesh=True,
            forward_axis='Y',
            up_axis='Z'
        )
    except AttributeError:
        # Fall back to old API (Blender 2.8-2.9x)
        bpy.ops.export_scene.obj(
            filepath=output_path,
            use_selection=True,
            use_materials=False,
            use_triangles=True,
            axis_forward='Y',
            axis_up='Z'
        )
    
    print(f"Exported to: {output_path}")

if __name__ == "__main__":
    # Remove Blender's default arguments
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []
    
    if len(argv) < 4:
        print("Usage: blender --background --python render-obj.py -- <N> <voxel_path> <voxel_size> <output_path>")
        sys.exit(1)
    
    N = int(argv[0])
    voxel_path = argv[1]
    voxel_size = float(argv[2])
    output_path = argv[3]
    
    voxel_to_obj(N, voxel_path, voxel_size, output_path)
    print(f"Conversion complete!")