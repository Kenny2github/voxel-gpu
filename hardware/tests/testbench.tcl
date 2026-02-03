quit -sim
vlib work
vlog ../src/*.sv $1
vopt +acc testbench -o tb
vsim tb

log *
add wave -recursive *

# rely on testbench to $stop
run -all
