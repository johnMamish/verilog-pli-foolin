`timescale 1ns/100ps

`define ARRAY_SIZE 64

module cumsum_tb();
    integer i;
    reg [7:0] image_in [0:63];
    reg signed [15:0] DCT_out [0:63];
    initial begin
        $readmemh("test_mem.hex", image_in, 0, 63);
        for (i = 0; i < `ARRAY_SIZE; i = i + 1) begin
            $display("image_in[%d] = %h", i, image_in[i]);
        end

        $loeffler_dct(image_in, DCT_out);

        for (i = 0; i < `ARRAY_SIZE; i = i + 1) begin
            $display("result[%d] = %h", i, DCT_out[i]);
        end
        $finish;
    end
endmodule
