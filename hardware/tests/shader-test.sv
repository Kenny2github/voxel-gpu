import gpu::*;
`timescale 1ns / 100ps

module testbench #(
    parameter INDEX_BITS = 32,
    parameter COORD_BITS = 8,
    parameter PALETTE_BITS = 8,
    parameter FRACT_BITS = 8,
    parameter PIXEL_BITS = 8
) ();

  logic do_rasterize;
  logic do_shade;
  logic [COORD_BITS-1:0] voxel_x;
  logic [COORD_BITS-1:0] voxel_y;
  logic [COORD_BITS-1:0] voxel_z;
  logic [PALETTE_BITS-1:0] voxel_id;
  logic [PIXEL_BITS-1:0] palette_entry;
  logic signed [COORD_BITS+FRACT_BITS-1:0] cam_pos_x;
  logic signed [COORD_BITS+FRACT_BITS-1:0] cam_pos_y;
  logic signed [COORD_BITS+FRACT_BITS-1:0] cam_pos_z;
  logic signed [COORD_BITS+FRACT_BITS-1:0] cam_look_x;
  logic signed [COORD_BITS+FRACT_BITS-1:0] cam_look_y;
  logic signed [COORD_BITS+FRACT_BITS-1:0] cam_look_z;
  logic [INDEX_BITS-1:0] pixel_index;
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
    cam_pos_x = {8'd4, 8'd0};
    cam_pos_y = {8'd4, 8'd0};
    cam_pos_z = {8'd4, 8'd0};
    cam_look_x = {8'(-1), 8'd0};
    cam_look_y = {8'(-1), 8'd0};
    cam_look_z = {8'(-1), 8'd0};
    pixel_index = 32'b0;

    @(negedge clock);
    reset = 1'b0;

    do_rasterize = 1'b1;
    // rasterize first voxel
    voxel_x = 8'd0;
    voxel_y = 8'd0;
    voxel_z = 8'd0;
    voxel_id = 8'd1;
    @(posedge rasterizing_done);
    @(posedge clock);
    // rasterize second voxel
    voxel_x  = 8'd2;
    voxel_y  = 8'd2;
    voxel_z  = 8'd2;
    voxel_id = 8'd2;
    @(posedge rasterizing_done);
    @(posedge clock);
    // rasterize first voxel a second time just for kicks
    // (to confirm it's actually checking min distance
    // and not just the last voxel rasterized)
    voxel_x  = 8'd0;
    voxel_y  = 8'd0;
    voxel_z  = 8'd0;
    voxel_id = 8'd1;
    @(posedge rasterizing_done);
    @(posedge clock);
    do_rasterize = 1'b0;

    // shade first voxel
    voxel_id = 8'd1;
    palette_entry = 8'h11;
    do_shade = 1'b1;
    @(posedge shading_done);
    @(posedge clock);
    // shade second voxel
    voxel_id = 8'd2;
    palette_entry = 8'h22;
    @(posedge shading_done);
    @(posedge clock);

    // one more cycle for good measure
    do_shade = 1'b0;
    @(posedge clock);

    $stop;
  end
endmodule
