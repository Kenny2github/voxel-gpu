import gpu::*;
`timescale 1ns / 100ps

module testbench #(
    parameter WIDTH_IN = 20,
    parameter WIDTH_CALC = 28,
    parameter WIDTH_OUT = 20,
    parameter FBITS = 10,
    parameter H_RESOLUTION = 320,
    parameter V_RESOLUTION = 240
) ();

  logic signed [WIDTH_IN-1:0] p0_x;
  logic signed [WIDTH_IN-1:0] p0_y;
  logic signed [WIDTH_IN-1:0] p0_z;
  logic signed [WIDTH_IN-1:0] p1_x;
  logic signed [WIDTH_IN-1:0] p1_y;
  logic signed [WIDTH_IN-1:0] p1_z;
  logic signed [WIDTH_IN-1:0] p2_x;
  logic signed [WIDTH_IN-1:0] p2_y;
  logic signed [WIDTH_IN-1:0] p2_z;
  logic signed [WIDTH_IN-1:0] p3_x;
  logic signed [WIDTH_IN-1:0] p3_y;
  logic signed [WIDTH_IN-1:0] p3_z;
  logic signed [$clog2(H_RESOLUTION)-1:0] x;
  logic signed [$clog2(V_RESOLUTION)-1:0] y;
  logic signed [WIDTH_OUT-1:0] val_x;
  logic signed [WIDTH_OUT-1:0] val_y;
  logic signed [WIDTH_OUT-1:0] val_z;
  logic start;
  logic done;
  logic error;
  logic clock;
  logic reset;

  lerp2_vec3 #(
      .WIDTH_IN(WIDTH_IN),
      .WIDTH_CALC(WIDTH_CALC),
      .WIDTH_OUT(WIDTH_OUT),
      .FBITS(FBITS),
      .X(H_RESOLUTION),
      .Y(V_RESOLUTION)
  ) DUT (
      .*
  );

  // set up clock
  initial begin
    clock <= 1'b0;
    forever begin
      #5 clock <= ~clock;
      if (error) $stop;
    end
  end

  initial begin
    reset = 1'b1;
    @(negedge clock);

    p0_x = 100 << FBITS;
    p0_y = 200 << FBITS;
    p0_z = 300 << FBITS;
    p1_x = 200 << FBITS;
    p1_y = 300 << FBITS;
    p1_z = 400 << FBITS;
    p2_x = 250 << FBITS;
    p2_y = 350 << FBITS;
    p2_z = 450 << FBITS;
    p3_x = 400 << FBITS;
    p3_y = 500 << FBITS;
    p3_z = 600 << FBITS;
    x = 12;
    y = 15;
    start = 1'b0;

    @(negedge clock);
    reset = 1'b0;

    start = 1'b1;
    @(posedge clock);
    start = 1'b0;
    @(posedge clock);
    @(posedge done);
    @(posedge clock);
    @(posedge clock);

    $stop;
  end
endmodule
