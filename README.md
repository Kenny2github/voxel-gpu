# Voxel GPU on an FPGA
Investigating the performance of a voxel-based GPU, by prototyping one on an FPGA. Conventional GPUs and their supporting software/firmware in the modern era are optimized for triangle rendering for several good reasons, but some of the reasons are historical accident. We do not necessarily expect to outperform conventional GPUs, but we hope to find out how close one can get.

The complexity of the project will likely come from multiple sources:
- The video protocol, for the actual rendering
- The FPGA design itself
- The software tooling to support the design (basically an OpenGL equivalent)
- The software used to demonstrate all the above
