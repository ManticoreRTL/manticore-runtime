module XorShift128 #(parameter SEED = 1481231)(
    input wire clock,
    output wire [31:0] rnd
);

    reg[31:0] x0 = 32'd12345678, x1 = 32'd36243669, x2 = 32'd521288629, x3 = SEED;
    reg[31:0] t, s;

    always@(posedge clock) begin

        t = x3;
        s = x0;
        x3 = x2;
        x2 = x1;
        x1 = s;
        t = t ^ (t << 11);
        t = t ^ (t >> 8);
        x0 = t ^ s ^ (s >> 19);
    end
    assign rnd = x0;

endmodule

module RandomAccess #(parameter ADDR_BITS = 5)(input wire clock);

    localparam TEST_SIZE = 1 << 24; // 16 Mi cycles


    wire [31:0] rnd;
    reg  [31:0] rnd_r;
    reg         vld_r = 0;
    reg  [31:0] counter = 0;
    XorShift128 rgen (.clock(clock), .rnd(rnd));

    reg [15:0] memory [0 : (1 << ADDR_BITS) - 1];

    wire [ADDR_BITS - 1 : 0] addr = rnd;
    reg  [ADDR_BITS - 1 : 0] addr_r;
    wire [15 : 0] rdata = memory[addr_r];
    always @(posedge clock) begin

        memory[addr] <= rnd[15:0];
        rnd_r  <= rnd;
        addr_r <= addr;
        vld_r  <= 1'b1;
        if (vld_r && rdata != rnd_r[15:0]) begin
            $display("Expected %d at %d but got %d", rnd_r[15:0], addr_r, rdata);
            $stop;
        end
        counter <= counter + 1;
        if (counter == TEST_SIZE) begin
            $finish;
        end

    end


endmodule