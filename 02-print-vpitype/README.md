# 02-array-print-vpitype

In its internals, Verilog has a vpiType for every entity that can concievably make its way to a Verilog PLI function. The list is surprisingly long and can be found in a C-code listing starting on page 523 if IEEE 1364-2005 (or found in "vpi_user.h".

This exposes one of the coolest things about the Verilog PLI (and one of my main reasons for going after it) - which is that you can pass arrays into and out of Verilog tasks written in C and using the Verilog PLI. For some reason, you CAN'T do this with Verilog tasks or functions written in Verilog, which can make it difficult to write behavioral testbenches to check the correctness of a block that we expect to transform a memory buffer.

In example 03, you can see a 1-d Verilog memory array actually being manipulated.

After compiling, the testbench can be run with

```
    vvp -M. -mprint_vpitype print_vpitype.vvp
```