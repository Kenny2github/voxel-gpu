# Voxel GPU on an FPGA
Investigating the performance of a voxel-based GPU, by prototyping one on an FPGA. Conventional GPUs and their supporting software/firmware in the modern era are optimized for triangle rendering for several good reasons, but some of the reasons are historical accident. We do not necessarily expect to outperform conventional GPUs, but we hope to find out how close one can get.

The complexity of the project will likely come from multiple sources:
- The video protocol, for the actual rendering
- The FPGA design itself
- The software tooling to support the design (basically an OpenGL equivalent)
- The software used to demonstrate all the above

## Requirements
For building:
- Linux system
- `make`
- [Quartus Prime Lite Edition version 21.1](https://www.intel.com/content/www/us/en/software-kit/684215/intel-quartus-prime-lite-edition-design-software-version-21-1-for-linux.html) installed in `~/intelFPGA_lite/21.1`
- Monitor Program version 18.1 from [FPGAcademy](https://fpgacademy.org/tools.html) installed in `~/intelFPGA_lite/21.1/`
- Optionally, for `clangd` support:
	- `python3`, `clang`, and `clangd`
	- Run `make cc` on clone
	- IDE support to pass `--query-driver=**/arm-altera-eabi-gcc` as an argument to `clangd`

For running:
- Any system
- Monitor Program version 18.1

## Build
1. Open `hardware/DE1_SoC_Computer.qpf` in Quartus and compile.
1. Simultaneously, `make`

## Run
1. Open the Monitor Program and use it to open `voxel_gpu.amp`
1. When prompted to "download the system", do so.
1. Load (do not compile, you already did that) the program.
1. Click Continue (or otherwise use the debugger).
