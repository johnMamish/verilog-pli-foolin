`timescale 1ns/100ps

module showvalue_tb();
    integer i;
    reg clock;
    reg [7:0] mynums [0:23];
    reg [7:0] mynums_out [0:23];
    initial begin
        $print_vpitype(mynums, mynums_out, i, showvalue_tb);
        $finish;
    end
endmodule
