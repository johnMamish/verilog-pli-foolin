all: testbench.v hello.c
	iverilog-vpi hello.c
# Alternatively, the following 2 lines achieve the same effect
#	gcc -c -fpic hello.c
#	gcc -shared -o hello.vpi hello.o -lvpi
	iverilog -o hello.vvp testbench.v
