module HelloManticore(input wire clock);


    localparam COUNT_DOWN = 10;

    reg [3:0] counter = COUNT_DOWN;
    always @(posedge clock) begin

        $display("%d!", counter);
        if (counter == 1) begin
            $display("Hello Manticore!");
            $finish;
        end
        counter <= counter - 1;
    end


endmodule