all: print_vpitype_tb.v print_vpitype.c
	iverilog-vpi print_vpitype.c
	iverilog -o print_vpitype.vvp print_vpitype_tb.v
