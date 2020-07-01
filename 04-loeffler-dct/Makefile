all: loeffler_dct_tb.v loeffler_dct.c
	iverilog-vpi loeffler_dct.c
	iverilog -o loeffler_dct.vvp loeffler_dct_tb.v
