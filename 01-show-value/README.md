# 01-show-value

This example implements a VPI function which prints the value of a given signal.

showvalue.c also contains an example of how to count the number of arguments passed into a vpi function.k

After compiling, the testbench can be run with

```
    vvp -M. -mshowvalue showvalue.vvp
```