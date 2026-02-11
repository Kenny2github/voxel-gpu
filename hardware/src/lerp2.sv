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
    output logic error,
    input logic clock,
    input logic reset
);
  logic [WIDTH-1:0] lerp_x, lerp_y, lerp_xy;
  assign val = p0 + lerp_x + lerp_y + lerp_xy;
  wand _done;
  assign done = _done;
  wor _error;
  assign error = _error;

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
      .dbz(_error),
      .ovf(_error),
      .a(WIDTH'(((p1 - p0) * {{WIDTH{x[WIDTH-1]}}, x}) >> FBITS)),
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
      .dbz(_error),
      .ovf(_error),
      .a(WIDTH'(((p2 - p0) * {{WIDTH{y[WIDTH-1]}}, y}) >> FBITS)),
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
      .dbz(_error),
      .ovf(_error),
      .a(WIDTH'(((p0 - p1 + p3 - p2) * {{WIDTH{x[WIDTH-1]}}, x} * y) >> (2 * FBITS))),
      .b(WIDTH'(({{WIDTH{X[WIDTH-1]}}, X} * Y) >> FBITS)),
      .val(lerp_xy)
  );

endmodule
