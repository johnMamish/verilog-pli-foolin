# 00-hello

This is a basic "Hello world!"-style example. The Verilog just consists of a single file which calls $hello() from the C file.

After compiling, the testbench can be run with

```
    vvp -M. -mhello hello.vvp

    -M.        gives vvp a search path for vpi files
    -mhello    gives vvp the basename for our vpi file; it will be expanded to hello.vpi
    hello.vvp  is the name of the vvp file to run.
```
