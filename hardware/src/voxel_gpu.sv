`timescale 1 ps / 1 ps

module voxel_gpu #(
	parameter DEFAULT_BUFFER = 32'h0800_0000,
	parameter DEFAULT_BACK_BUFFER = 32'h0800_0000
) (
		input  wire [7:0]  s1_address,       //    s1.address
		output wire [31:0] s1_readdata,      //      .readdata
		input  wire [31:0] s1_writedata,     //      .writedata
		input  wire        s1_write,         //      .write
		output wire        s1_waitrequest,   //      .waitrequest
		input  wire        reset,            // reset.reset
		input  wire        clock,            // clock.clk
		output wire        irq,              //   irq.irq
		output wire [31:0] m1_address,       //    m1.address
		output wire [31:0] m1_writedata,     //      .writedata
		output wire        m1_write,         //      .write
		input  wire        m1_waitrequest,   //      .waitrequest
		input  wire [31:0] m1_readdata,      //      .readdata
		output wire        m1_read,          //      .read
		input  wire        m1_readdatavalid  //      .readdatavalid
	);

	logic [31:0] pixel_buffer1, pixel_buffer2;

	always_ff @(posedge clock, posedge reset) begin
		if (reset) begin
			pixel_buffer1 <= DEFAULT_BUFFER;
			pixel_buffer2 <= DEFAULT_BACK_BUFFER;
		end else if (s1_write) begin
			case (s1_address)
				8'h00: begin
					pixel_buffer1 <= s1_writedata;
				end
				8'h01: begin
					pixel_buffer2 <= s1_writedata;
				end
			endcase
		end
	end

	always_comb begin
		case (s1_address)
			8'h00: begin
				s1_readdata = pixel_buffer1;
			end
			8'h01: begin
				s1_readdata = pixel_buffer2;
			end
			default: begin
				s1_readdata = 32'b00000000000000000000000000000000;
			end
		endcase
	end

	assign s1_waitrequest = 1'b0;
	assign irq = 1'b0;

endmodule
