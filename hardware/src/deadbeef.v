`default_nettype wire

module deadbeef #(
	parameter VALUE = 32'hDEADBEEF
) (
	input clock,
	input reset,
	output [31:0] s1_readdata
);

	assign s1_readdata = VALUE;

endmodule
