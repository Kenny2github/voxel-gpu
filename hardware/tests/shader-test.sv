import gpu::*;
`timescale 1ns / 100ps

module testbench #(
    parameter ROW_BITS = 8,
    parameter COL_BITS = 8,
    parameter COORD_BITS = 8,
    parameter PALETTE_BITS = 8,
    parameter FRAC_BITS = 8,
    parameter PIXEL_BITS = 8
) ();

  logic do_rasterize;
  logic do_shade;
  logic [COORD_BITS-1:0] voxel_x;
  logic [COORD_BITS-1:0] voxel_y;
  logic [COORD_BITS-1:0] voxel_z;
  logic [PALETTE_BITS-1:0] voxel_id;
  logic [PIXEL_BITS-1:0] palette_entry;
  logic signed [COORD_BITS+FRAC_BITS-1:0] cam_pos_x;
  logic signed [COORD_BITS+FRAC_BITS-1:0] cam_pos_y;
  logic signed [COORD_BITS+FRAC_BITS-1:0] cam_pos_z;
  logic signed [COORD_BITS+FRAC_BITS-1:0] cam_look_x;
  logic signed [COORD_BITS+FRAC_BITS-1:0] cam_look_y;
  logic signed [COORD_BITS+FRAC_BITS-1:0] cam_look_z;
  logic [ROW_BITS-1:0] row;
  logic [COL_BITS-1:0] col;
  logic rasterizing_done;
  logic shading_done;
  wire [PIXEL_BITS-1:0] pixel;
  logic reset;
  logic clock;

  pixel_shader #() DUT (.*);

  // set up clock
  initial begin
    clock <= 1'b0;
    forever begin
      #5 clock <= ~clock;
    end
  end

  initial begin
    reset = 1'b1;
    @(negedge clock);

    do_rasterize = 1'b0;
    do_shade = 1'b0;
    voxel_x = 8'b0;
    voxel_y = 8'b0;
    voxel_z = 8'b0;
    voxel_id = 8'b1;
    palette_entry = 8'hff;
    cam_pos_x = {8'd1, 8'd0};
    cam_pos_y = {8'd1, 8'd0};
    cam_pos_z = {8'd1, 8'd0};
    cam_look_x = {8'(-1), 8'd0};
    cam_look_y = {8'(-1), 8'd0};
    cam_look_z = {8'(-1), 8'd0};
    row = 8'b0;
    col = 8'b0;

    @(negedge clock);
    reset = 1'b0;

    do_rasterize = 1'b1;
    @(posedge rasterizing_done);
    do_rasterize = 1'b0;
    @(negedge clock);
    do_shade = 1'b1;
    @(posedge shading_done);
    do_shade = 1'b0;
    @(posedge clock);

    $stop;
  end
endmodule
