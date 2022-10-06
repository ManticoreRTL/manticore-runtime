// make the ram fit in scratch pad
module RandomAccessExtraLarge(input wire clock);
    RandomAccess #(.ADDR_BITS(18)) tester (.clock(clock));
endmodule