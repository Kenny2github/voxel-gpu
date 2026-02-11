#
# request TCL package from ACDS 16.1
#
package require -exact qsys 16.1


#
# module voxel_gpu
#
set_module_property DESCRIPTION ""
set_module_property NAME voxel_gpu
set_module_property VERSION 0.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR ""
set_module_property DISPLAY_NAME "Voxel GPU"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property REPORT_HIERARCHY false


#
# file sets
#
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL voxel_gpu
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file gpu.sv SYSTEM_VERILOG PATH gpu.sv
add_fileset_file voxel_gpu.sv SYSTEM_VERILOG PATH voxel_gpu.sv TOP_LEVEL_FILE
add_fileset_file gpu_controller.sv SYSTEM_VERILOG PATH gpu_controller.sv
add_fileset_file pixel_shader.sv SYSTEM_VERILOG PATH pixel_shader.sv


#
# parameters
#
add_parameter H_RESOLUTION INTEGER 256 ""
set_parameter_property H_RESOLUTION DEFAULT_VALUE 256
set_parameter_property H_RESOLUTION DISPLAY_NAME "Horizontal Resolution"
set_parameter_property H_RESOLUTION TYPE INTEGER
set_parameter_property H_RESOLUTION UNITS None
set_parameter_property H_RESOLUTION DISPLAY_UNITS pixels
set_parameter_property H_RESOLUTION ALLOWED_RANGES -2147483648:2147483647
set_parameter_property H_RESOLUTION DESCRIPTION ""
set_parameter_property H_RESOLUTION HDL_PARAMETER true
add_parameter V_RESOLUTION INTEGER 192 ""
set_parameter_property V_RESOLUTION DEFAULT_VALUE 192
set_parameter_property V_RESOLUTION DISPLAY_NAME "Vertical Resolution"
set_parameter_property V_RESOLUTION TYPE INTEGER
set_parameter_property V_RESOLUTION UNITS None
set_parameter_property V_RESOLUTION DISPLAY_UNITS pixels
set_parameter_property V_RESOLUTION ALLOWED_RANGES -2147483648:2147483647
set_parameter_property V_RESOLUTION DESCRIPTION ""
set_parameter_property V_RESOLUTION HDL_PARAMETER true
add_parameter PIXEL_BITS INTEGER 16 ""
set_parameter_property PIXEL_BITS DEFAULT_VALUE 16
set_parameter_property PIXEL_BITS DISPLAY_NAME "Bits per pixel"
set_parameter_property PIXEL_BITS TYPE INTEGER
set_parameter_property PIXEL_BITS UNITS Bits
set_parameter_property PIXEL_BITS ALLOWED_RANGES -2147483648:2147483647
set_parameter_property PIXEL_BITS DESCRIPTION ""
set_parameter_property PIXEL_BITS HDL_PARAMETER true


#
# display items
#


#
# connection point s1
#
add_interface s1 avalon end
set_interface_property s1 addressUnits WORDS
set_interface_property s1 associatedClock clock
set_interface_property s1 associatedReset reset
set_interface_property s1 bitsPerSymbol 8
set_interface_property s1 burstOnBurstBoundariesOnly false
set_interface_property s1 burstcountUnits WORDS
set_interface_property s1 explicitAddressSpan 0
set_interface_property s1 holdTime 0
set_interface_property s1 linewrapBursts false
set_interface_property s1 maximumPendingReadTransactions 0
set_interface_property s1 maximumPendingWriteTransactions 0
set_interface_property s1 readLatency 0
set_interface_property s1 readWaitTime 1
set_interface_property s1 setupTime 0
set_interface_property s1 timingUnits Cycles
set_interface_property s1 writeWaitTime 0
set_interface_property s1 ENABLED true
set_interface_property s1 EXPORT_OF ""
set_interface_property s1 PORT_NAME_MAP ""
set_interface_property s1 CMSIS_SVD_VARIABLES ""
set_interface_property s1 SVD_ADDRESS_GROUP ""

add_interface_port s1 s1_address address Input 8
add_interface_port s1 s1_readdata readdata Output 32
add_interface_port s1 s1_writedata writedata Input 32
add_interface_port s1 s1_write write Input 1
add_interface_port s1 s1_waitrequest waitrequest Output 1
add_interface_port s1 s1_read read Input 1
set_interface_assignment s1 embeddedsw.configuration.isFlash 0
set_interface_assignment s1 embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment s1 embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment s1 embeddedsw.configuration.isPrintableDevice 0


#
# connection point reset
#
add_interface reset reset end
set_interface_property reset associatedClock clock
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF ""
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset CMSIS_SVD_VARIABLES ""
set_interface_property reset SVD_ADDRESS_GROUP ""

add_interface_port reset reset reset Input 1


#
# connection point clock
#
add_interface clock clock end
set_interface_property clock clockRate 0
set_interface_property clock ENABLED true
set_interface_property clock EXPORT_OF ""
set_interface_property clock PORT_NAME_MAP ""
set_interface_property clock CMSIS_SVD_VARIABLES ""
set_interface_property clock SVD_ADDRESS_GROUP ""

add_interface_port clock clock clk Input 1


#
# connection point irq
#
add_interface irq interrupt end
set_interface_property irq associatedAddressablePoint ""
set_interface_property irq associatedClock clock
set_interface_property irq associatedReset reset
set_interface_property irq bridgedReceiverOffset ""
set_interface_property irq bridgesToReceiver ""
set_interface_property irq ENABLED true
set_interface_property irq EXPORT_OF ""
set_interface_property irq PORT_NAME_MAP ""
set_interface_property irq CMSIS_SVD_VARIABLES ""
set_interface_property irq SVD_ADDRESS_GROUP ""

add_interface_port irq irq irq Output 1


#
# connection point m1
#
add_interface m1 avalon start
set_interface_property m1 addressUnits SYMBOLS
set_interface_property m1 associatedClock clock
set_interface_property m1 associatedReset reset
set_interface_property m1 bitsPerSymbol 8
set_interface_property m1 burstOnBurstBoundariesOnly false
set_interface_property m1 burstcountUnits WORDS
set_interface_property m1 doStreamReads false
set_interface_property m1 doStreamWrites false
set_interface_property m1 holdTime 0
set_interface_property m1 linewrapBursts false
set_interface_property m1 maximumPendingReadTransactions 0
set_interface_property m1 maximumPendingWriteTransactions 0
set_interface_property m1 readLatency 0
set_interface_property m1 readWaitTime 1
set_interface_property m1 setupTime 0
set_interface_property m1 timingUnits Cycles
set_interface_property m1 writeWaitTime 0
set_interface_property m1 ENABLED true
set_interface_property m1 EXPORT_OF ""
set_interface_property m1 PORT_NAME_MAP ""
set_interface_property m1 CMSIS_SVD_VARIABLES ""
set_interface_property m1 SVD_ADDRESS_GROUP ""

add_interface_port m1 m1_address address Output 32
add_interface_port m1 m1_writedata writedata Output 16
add_interface_port m1 m1_write write Output 1
add_interface_port m1 m1_waitrequest waitrequest Input 1
