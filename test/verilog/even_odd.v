module EvenOdd(input wire clock);

    reg [15:0] counter = 0;

    always @(posedge clock) begin

        if (counter[0] == 1'b0) begin
            $display("%d is an even number", counter);
        end else begin
            $display("%d is an odd number", counter);
        end
        counter <= counter + 1;
        if (counter == 20) begin
            $finish;
        end
    end

endmodule