import gpu::*;
`timescale 1ns / 100ps

module testbench #(
    parameter WIDTH = 32,
    parameter FBITS = 16,
    parameter H_RESOLUTION = 320,
    parameter V_RESOLUTION = 240
) ();

  logic signed [WIDTH-1:0] p0;
  logic signed [WIDTH-1:0] p1;
  logic signed [WIDTH-1:0] p2;
  logic signed [WIDTH-1:0] p3;
  logic signed [WIDTH-1:0] x;
  logic signed [WIDTH-1:0] y;
  logic signed [WIDTH-1:0] val;
  logic start;
  logic done;
  logic error;
  logic clock;
  logic reset;

  lerp2 #(
      .WIDTH(WIDTH),
      .FBITS(FBITS),
      .X(WIDTH'((H_RESOLUTION - 1) << FBITS)),
      .Y(WIDTH'((V_RESOLUTION - 1) << FBITS))
  ) DUT (.*);

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

    p0 = WIDTH'(100 << FBITS);
    p1 = WIDTH'(200 << FBITS);
    p2 = WIDTH'(250 << FBITS);
    p3 = WIDTH'(400 << FBITS);
    x = WIDTH'(12 << FBITS);
    y = WIDTH'(15 << FBITS);
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
