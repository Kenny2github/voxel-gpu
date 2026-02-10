module lerp2 #(
    parameter WIDTH = 32,
    parameter FBITS = 16
) (
    input logic signed [WIDTH-1:0] p0,
    input logic signed [WIDTH-1:0] p1,
    input logic signed [WIDTH-1:0] p2,
    input logic signed [WIDTH-1:0] p3,
    input logic signed [WIDTH-1:0] x,
    input logic signed [WIDTH-1:0] y,
    input logic signed [WIDTH-1:0] X,
    input logic signed [WIDTH-1:0] Y,
    output logic signed [WIDTH-1:0] val,
    input logic start,
    output logic done,
    input logic clock,
    input logic reset
);
  logic [WIDTH-1:0] lerp_x, lerp_y, lerp_xy;
  assign val = p0 + lerp_x + lerp_y + lerp_xy;
  wand _done;
  assign done = _done;

  div #(
      .WIDTH(WIDTH),
      .FBITS(FBITS)
  ) div_x (
      .clk(clock),
      .rst(reset),
      .start(start),
      .valid(_done),
      .busy(),
      .done(_done),
      .dbz(),
      .ovf(),
      .a((p1 - p0) * x),
      .b(X),
      .val(lerp_x)
  );
  div #(
      .WIDTH(WIDTH),
      .FBITS(FBITS)
  ) div_y (
      .clk(clock),
      .rst(reset),
      .start(start),
      .valid(_done),
      .busy(),
      .done(_done),
      .dbz(),
      .ovf(),
      .a((p2 - p0) * y),
      .b(Y),
      .val(lerp_y)
  );
  div #(
      .WIDTH(WIDTH),
      .FBITS(FBITS)
  ) div_xy (
      .clk(clock),
      .rst(reset),
      .start(start),
      .valid(_done),
      .busy(),
      .done(_done),
      .dbz(),
      .ovf(),
      .a((p0 - p1 + p3 - p2) * x * y),
      .b(X * Y),
      .val(lerp_xy)
  );

endmodule
