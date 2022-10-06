// make the ram fit in scratch pad
module RandomAccessLarge(input wire clock);
    RandomAccess #(.ADDR_BITS(15)) tester (.clock(clock));
endmodule