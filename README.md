# Verilog PLI (Programming Language Interface) examples
The Verilog PLI is a way that C programs can interface with Verilog programs.


### What's in this repository
Each subfolder contains a self-contained Verilog PLI example which can be compiled with make. When I haven't used Verilog PLI in 3 years, I plan to come back here and copy-paste examples and makefiles.

Subfolders prefixed with xx- are not yet implemented, but are ideas for future.


### How is the Verilog PLI useful?
I bet I don't even know 5% of the ways that you can use the Verilog PLI to creatively extend the language. As far as I can tell (and I bet I'm wrong), it's mostly useful in non-synthesizable testbench code. Here are some example use-cases I've run into:

   1. Writing a complicated, non-synthesizable, behavioral model.<br/>
      A common workflow for me is to sanity-check my understanding of an algorithm by first implementing it in C (for instance, a JPEG compressor), and then designing hardware to implement the same algorithm. As I test the hardware, I'd like to be able to compare its output to the output of my C implementation. Without the Verilog PLI, you're stuck either re-implementing your C algorithm in procedural testbench Verilog (which lacks some major features, making anything but the most basic algorithms unwieldy to implement) or running both your C program and your testbench and doing some sort of file comparison on the output files.

   2. Extending tools like gtkwave
