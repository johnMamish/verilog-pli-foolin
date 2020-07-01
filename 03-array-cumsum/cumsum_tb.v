`timescale 1ns/100ps

`define ARRAY_SIZE 16

module showvalue_tb();
    integer i;
    reg  clock_prev;
    reg [7:0] mynums [0:(`ARRAY_SIZE - 1)];
    reg [7:0] mynums_out [0:(`ARRAY_SIZE - 1)];
    initial begin

        for (i = 0; i < `ARRAY_SIZE; i = i + 1) begin
            mynums[i] = (2 * i) + 1;
        end

        $cumsum(mynums, mynums_out);

        for (i = 0; i < `ARRAY_SIZE; i = i + 1) begin
            $display("result[%d] = %d", i, mynums_out[i]);
        end
        $finish;
    end
endmodule
