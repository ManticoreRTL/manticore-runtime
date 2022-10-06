// make fifo larger than the cache
module FifoExtraLarge(input wire clock);
    FifoTester #(.ADDR_BITS(18)) tester (.clock(clock));
endmodule
