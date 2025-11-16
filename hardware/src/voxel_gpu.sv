`include "common.svh"

module voxel_gpu #(
	parameter DEFAULT_BUFFER = 32'h0800_0000,
	parameter H_RESOLUTION = 16'd256,
	parameter V_RESOLUTION = 16'd192
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

	logic [31:0]
		// GPU.pixel_buffer
		pixel_buffer,
		// GPU.voxel_buffer
		voxel_buffer;
	// GPU.camera
	camera cam;

	always_ff @(posedge clock or posedge reset) begin
		if (reset) begin
			pixel_buffer <= DEFAULT_BUFFER;
			voxel_buffer <= '0;
			cam <= '{default: 0};
		end else if (s1_write) begin
			case (s1_address)
				8'h00: begin
					pixel_buffer <= s1_writedata;
				end
				8'h01: begin
					voxel_buffer <= s1_writedata;
				end
				8'h02: begin
					cam.pos.x <= s1_writedata;
				end
				8'h03: begin
					cam.pos.y <= s1_writedata;
				end
				8'h04: begin
					cam.pos.z <= s1_writedata;
				end
				8'h05: begin
					cam.look0.x <= s1_writedata;
				end
				8'h06: begin
					cam.look0.y <= s1_writedata;
				end
				8'h07: begin
					cam.look0.z <= s1_writedata;
				end
				8'h08: begin
					cam.look1.x <= s1_writedata;
				end
				8'h09: begin
					cam.look1.y <= s1_writedata;
				end
				8'h0a: begin
					cam.look1.z <= s1_writedata;
				end
				8'h0b: begin
					cam.look2.x <= s1_writedata;
				end
				8'h0c: begin
					cam.look2.y <= s1_writedata;
				end
				8'h0d: begin
					cam.look2.z <= s1_writedata;
				end
			endcase
		end
	end

	always_comb begin
		case (s1_address)
			8'h00: begin
				s1_readdata = pixel_buffer;
			end
			8'h01: begin
				s1_readdata = voxel_buffer;
			end
			8'h02: begin
				s1_readdata = cam.pos.x;
			end
			8'h03: begin
				s1_readdata = cam.pos.y;
			end
			8'h04: begin
				s1_readdata = cam.pos.z;
			end
			8'h05: begin
				s1_readdata = cam.look0.x;
			end
			8'h06: begin
				s1_readdata = cam.look0.y;
			end
			8'h07: begin
				s1_readdata = cam.look0.z;
			end
			8'h08: begin
				s1_readdata = cam.look1.x;
			end
			8'h09: begin
				s1_readdata = cam.look1.y;
			end
			8'h0a: begin
				s1_readdata = cam.look1.z;
			end
			8'h0b: begin
				s1_readdata = cam.look2.x;
			end
			8'h0c: begin
				s1_readdata = cam.look2.y;
			end
			8'h0d: begin
				s1_readdata = cam.look2.z;
			end
			default: begin
				s1_readdata = 32'b0;
			end
		endcase
	end

	assign s1_waitrequest = 1'b0;
	assign m1_write = 1'b0;
	assign m1_writedata = 32'b0;
	assign m1_address = 32'b0;
	assign m1_read = 1'b0;
	assign irq = 1'b0;

endmodule
