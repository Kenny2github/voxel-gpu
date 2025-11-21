`include "common.svh"

module pixel_shader #(
    parameter ROW = 0,
    parameter COL = 0,
    parameter ROW_BITS = 8,
    parameter COL_BITS = 8,
    parameter COORD_BITS = 8,
    parameter PALETTE_BITS = 8,
		parameter FRAC_BITS = 8,
    parameter PIXEL_BITS = 8
) (
    input logic do_rasterize,
    input logic do_shade,
    input logic [COORD_BITS-1:0] voxel_x,
    input logic [COORD_BITS-1:0] voxel_y,
    input logic [COORD_BITS-1:0] voxel_z,
    input logic [PALETTE_BITS-1:0] voxel_id,
    input logic [31:0] cam_pos_x,
    input logic [31:0] cam_pos_y,
    input logic [31:0] cam_pos_z,
    input logic [31:0] cam_look_x,
    input logic [31:0] cam_look_y,
    input logic [31:0] cam_look_z,
    input logic [ROW_BITS-1:0] row,
    input logic [COL_BITS-1:0] col,
    output logic rasterizing_done,
    output logic shading_done,
    output wire [PIXEL_BITS-1:0] pixel,
    input logic reset,
    input logic clock
);
  enum logic [3:0] {
    IDLE,
    MEASURE,
    STORE,
    DONE
  }
      state, next_state;

  logic [PIXEL_BITS-1:0] _pixel;
  assign pixel = (row == ROW && col == COL) ? _pixel : 'z;

	assign tAx = ({voxel_x, 8'b0} - cam_pos_x) / Dx;
	assign tAy = ({voxel_y, 8'b0} - cam_pos_y) / Dy;
	assign tAz = ({voxel_z, 8'b0} - cam_pos_z) / Dz;
	assign tBx = ({voxel_x + 1'b1, 8'b0} - cam_pos_x) / Dx;
	assign tBy = ({voxel_y + 1'b1, 8'b0} - cam_pos_y) / Dy;
	assign tBz = ({voxel_z + 1'b1, 8'b0} - cam_pos_z) / Dz;

  always_ff @(posedge clock, posedge reset) begin
    if (reset) begin
      state <= IDLE;
    end else begin
      state <= next_state;
    end
  end

  always_comb begin
    case (state)
      IDLE: begin
        if (valid) next_state = MEASURE;
        else next_state = IDLE;
      end
      MEASURE: begin
        next_state = STORE;
      end
      STORE: begin
        next_state = DONE;
      end
      DONE: begin
        next_state = IDLE;
      end
    endcase
  end

endmodule
