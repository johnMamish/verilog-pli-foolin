`timescale 1ns/100ps

`define ARRAY_SIZE 64

module cumsum_tb();
    integer i;
    reg [7:0] image_in [0:63];
    reg signed [15:0] DCT_out [0:63];
    initial begin

        for (i = 0; i < `ARRAY_SIZE; i = i + 1) begin
            image_in[i] = 64 + ((i % 2) * 128);
        end
        $loeffler_dct(image_in, DCT_out, 8, 8);

        for (i = 0; i < `ARRAY_SIZE; i = i + 1) begin
            $display("result[%d] = %d", i, DCT_out[i]);
        end
        $finish;
    end
endmodule
