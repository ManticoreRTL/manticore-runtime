// make the ram fit in scratch pad
module RandomAccessSmall(input wire clock);
    RandomAccess #(.ADDR_BITS(9)) tester (.clock(clock));
endmodule