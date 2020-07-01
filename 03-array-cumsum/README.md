# 02-array-cumsum

This example implements a VPI function which calculates the running cumulative sum of a 1-d array into a second array (like the cumsum function in numpy).

After compiling, the testbench can be run with

```
    vvp -M. -mcumsum cumsum.vvp
```