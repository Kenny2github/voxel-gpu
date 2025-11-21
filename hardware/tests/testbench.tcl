quit -sim
vlib work
vlog ../src/*.sv *.sv
vopt +acc testbench -o tb
vsim tb
when -fast {/testbench/done=1} {
	stop
}
run -all
