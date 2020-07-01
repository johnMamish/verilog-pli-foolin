all: cumsum_tb.v cumsum.c
	iverilog-vpi cumsum.c
	iverilog -o cumsum.vvp cumsum_tb.v
