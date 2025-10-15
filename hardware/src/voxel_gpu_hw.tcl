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
add_fileset_file voxel_gpu.sv VERILOG PATH voxel_gpu.sv TOP_LEVEL_FILE


#
# parameters
#
add_parameter DEFAULT_BUFFER STD_LOGIC_VECTOR 134217728 ""
set_parameter_property DEFAULT_BUFFER DEFAULT_VALUE 134217728
set_parameter_property DEFAULT_BUFFER DISPLAY_NAME "Default Pixel Buffer Address"
set_parameter_property DEFAULT_BUFFER WIDTH 32
set_parameter_property DEFAULT_BUFFER TYPE STD_LOGIC_VECTOR
set_parameter_property DEFAULT_BUFFER UNITS None
set_parameter_property DEFAULT_BUFFER DESCRIPTION ""
set_parameter_property DEFAULT_BUFFER HDL_PARAMETER true
add_parameter DEFAULT_BACK_BUFFER STD_LOGIC_VECTOR 134217728 ""
set_parameter_property DEFAULT_BACK_BUFFER DEFAULT_VALUE 134217728
set_parameter_property DEFAULT_BACK_BUFFER DISPLAY_NAME "Default Back Buffer Address"
set_parameter_property DEFAULT_BACK_BUFFER WIDTH 32
set_parameter_property DEFAULT_BACK_BUFFER TYPE STD_LOGIC_VECTOR
set_parameter_property DEFAULT_BACK_BUFFER UNITS None
set_parameter_property DEFAULT_BACK_BUFFER DESCRIPTION ""
set_parameter_property DEFAULT_BACK_BUFFER HDL_PARAMETER true

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
add_interface_port m1 m1_writedata writedata Output 32
add_interface_port m1 m1_write write Output 1
add_interface_port m1 m1_waitrequest waitrequest Input 1
add_interface_port m1 m1_readdata readdata Input 32
add_interface_port m1 m1_read read Output 1
add_interface_port m1 m1_readdatavalid readdatavalid Input 1
