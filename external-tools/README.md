# convert-obj.py
python convert-obj.py [N] [WN] [obj_path] [ox] [oy] [oz] [output_path]

- N: voxel size of object (NxNxN at most)
- WN: world voxel size 
- obj_path: file path of object to convert
- ox, oy, oz: center position of x, y, z
- output_path: file path of voxel output (.voxel)

# convert-obj.py
blender --background --python render-obj.py -- [WN] [voxel_path] [voxel_size] [output_path]

- N: voxel size of world
- voxel_path: file path of voxel data to convert
- voxel size: Size of voxel in model space
- output_path: file path of object output (.obj)

* Known to work with Blender 4.0