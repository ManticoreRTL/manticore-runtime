// make fifo larger than available local memory, but smaller than cache
module FifoLargeTest(input wire clock);
    FifoTester #(.ADDR_BITS(15)) tester (.clock(clock));
endmodule