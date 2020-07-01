`timescale 1ns/100ps

module showvalue_tb();
    reg clock;

    ////////////////////////////////
    // Set up a shift register
    reg [15:0] shifter;
    always @(posedge clock) begin
        shifter <= { shifter[14:0], 1'h0 };
    end
    initial shifter = 16'hba11;

    always begin
        #500; clock = ~clock; #500;
    end

    integer i;
    reg  clock_prev;
    initial begin
        clock = 1'b0;
        for (i = 0; i < 16; i = i + 1) begin
            $showvalue(shifter);
            @(posedge clock);
            #100;
        end

        $finish;
    end
endmodule
