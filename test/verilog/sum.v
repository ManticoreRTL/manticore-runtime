module CheckSum(input wire clock);

    localparam TEST_SIZE = 16;
    reg [31:0] sum = 0;
    reg [31:0] reference = 0;

    reg [31:0] values [0 : TEST_SIZE - 1];

    integer i;
    initial begin
        reference = (TEST_SIZE - 1) * TEST_SIZE / 2;
        for (i = 0; i < TEST_SIZE; i = i + 1) begin
            // reference = reference + i;
            values[i] = i;
        end
    end


    reg [15:0] counter = 0;
    always @(posedge clock) begin

        counter <= counter + 1;
        sum <= sum + values[counter];

        // $display("%d + %d (%d)", sum, values[counter], counter);

        if (counter == TEST_SIZE) begin
            $display("Expected %d and got %d", reference, sum);
            assert(reference == sum);
            $finish;
        end

    end


endmodule