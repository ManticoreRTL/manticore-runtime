// make fifo small enough to fit in scratch pads
module FifoSmall(input wire clock);
    FifoTester #(.ADDR_BITS(9)) tester (.clock(clock));
endmodule