quit -sim
vlib work
vlog ../src/*.sv *.sv
vopt +acc testbench -o tb
vsim tb

log *
add wave -recursive *

when -fast {/testbench/done=1} {
	stop
}
run -all
