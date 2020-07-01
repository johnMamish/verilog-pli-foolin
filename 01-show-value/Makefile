all: showvalue_tb.v showvalue.c
	iverilog-vpi showvalue.c
	iverilog -o showvalue.vvp showvalue_tb.v
