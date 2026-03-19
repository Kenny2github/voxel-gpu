import gpu::*;

module lerp2_vec3 #(
    parameter WIDTH_IN = 32,
    parameter WIDTH_CALC = 32,
    parameter WIDTH_OUT = 24,
    parameter FBITS = 16,
    parameter X = 320,
    parameter Y = 240
) (
    input logic signed [WIDTH_IN-1:0] p0_x,
    input logic signed [WIDTH_IN-1:0] p0_y,
    input logic signed [WIDTH_IN-1:0] p0_z,
    input logic signed [WIDTH_IN-1:0] p1_x,
    input logic signed [WIDTH_IN-1:0] p1_y,
    input logic signed [WIDTH_IN-1:0] p1_z,
    input logic signed [WIDTH_IN-1:0] p2_x,
    input logic signed [WIDTH_IN-1:0] p2_y,
    input logic signed [WIDTH_IN-1:0] p2_z,
    input logic signed [WIDTH_IN-1:0] p3_x,
    input logic signed [WIDTH_IN-1:0] p3_y,
    input logic signed [WIDTH_IN-1:0] p3_z,
    input logic signed [$clog2(X)-1:0] x,
    input logic signed [$clog2(Y)-1:0] y,
    output logic signed [WIDTH_OUT-1:0] val_x,
    output logic signed [WIDTH_OUT-1:0] val_y,
    output logic signed [WIDTH_OUT-1:0] val_z,
    input logic start,
    output logic done,
    output logic error,
    input logic clock,
    input logic reset
);
  enum logic [3:0] {
    IDLE,
    RESET_X,
    START_X,
    LERP2_X,
    RESET_Y,
    START_Y,
    LERP2_Y,
    RESET_Z,
    START_Z,
    LERP2_Z,
    DONE,
    ERROR
  } state;

  logic signed [WIDTH_IN-1:0] lerp2_p0, lerp2_p1, lerp2_p2, lerp2_p3;
  logic signed [WIDTH_CALC-1:0] lerp2_val;
  logic lerp2_reset, lerp2_start, lerp2_done, lerp2_error;

  always_ff @(posedge clock or posedge reset) begin
    if (reset) begin
      state <= IDLE;
      val_x <= '0;
      val_y <= '0;
      val_z <= '0;
    end else begin
      case (state)
        IDLE: begin
          if (start) state <= RESET_X;
        end
        RESET_X: begin
          state <= START_X;
        end
        START_X: begin
          state <= LERP2_X;
        end
        LERP2_X: begin
          if (lerp2_done) begin
            state <= RESET_Y;
            val_x <= lerp2_val[WIDTH_OUT-1:0];
          end else if (lerp2_error) state <= ERROR;
        end
        RESET_Y: begin
          state <= START_Y;
        end
        START_Y: begin
          state <= LERP2_Y;
        end
        LERP2_Y: begin
          if (lerp2_done) begin
            state <= RESET_Z;
            val_y <= lerp2_val[WIDTH_OUT-1:0];
          end else if (lerp2_error) state <= ERROR;
        end
        RESET_Z: begin
          state <= START_Z;
        end
        START_Z: begin
          state <= LERP2_Z;
        end
        LERP2_Z: begin
          if (lerp2_done) begin
            state <= DONE;
            val_z <= lerp2_val[WIDTH_OUT-1:0];
          end else if (lerp2_error) state <= ERROR;
        end
        DONE: begin
          state <= IDLE;
        end
      endcase
    end
  end

  always_comb begin
    lerp2_p0 = '0;
    lerp2_p1 = '0;
    lerp2_p2 = '0;
    lerp2_p3 = '0;
    lerp2_reset = 1'b0;
    lerp2_start = 1'b0;
    done = 1'b0;
    error = 1'b0;
    case (state)
      RESET_X: begin
        lerp2_reset = 1'b1;
        lerp2_p0 = p0_x;
        lerp2_p1 = p1_x;
        lerp2_p2 = p2_x;
        lerp2_p3 = p3_x;
      end
      START_X: begin
        lerp2_start = 1'b1;
        lerp2_p0 = p0_x;
        lerp2_p1 = p1_x;
        lerp2_p2 = p2_x;
        lerp2_p3 = p3_x;
      end
      LERP2_X: begin
        lerp2_p0 = p0_x;
        lerp2_p1 = p1_x;
        lerp2_p2 = p2_x;
        lerp2_p3 = p3_x;
      end
      RESET_Y: begin
        lerp2_reset = 1'b1;
        lerp2_p0 = p0_y;
        lerp2_p1 = p1_y;
        lerp2_p2 = p2_y;
        lerp2_p3 = p3_y;
      end
      START_Y: begin
        lerp2_start = 1'b1;
        lerp2_p0 = p0_y;
        lerp2_p1 = p1_y;
        lerp2_p2 = p2_y;
        lerp2_p3 = p3_y;
      end
      LERP2_Y: begin
        lerp2_p0 = p0_y;
        lerp2_p1 = p1_y;
        lerp2_p2 = p2_y;
        lerp2_p3 = p3_y;
      end
      RESET_Z: begin
        lerp2_reset = 1'b1;
        lerp2_p0 = p0_z;
        lerp2_p1 = p1_z;
        lerp2_p2 = p2_z;
        lerp2_p3 = p3_z;
      end
      START_Z: begin
        lerp2_start = 1'b1;
        lerp2_p0 = p0_z;
        lerp2_p1 = p1_z;
        lerp2_p2 = p2_z;
        lerp2_p3 = p3_z;
      end
      LERP2_Z: begin
        lerp2_p0 = p0_z;
        lerp2_p1 = p1_z;
        lerp2_p2 = p2_z;
        lerp2_p3 = p3_z;
      end
      DONE: begin
        done = 1'b1;
      end
      ERROR: begin
        error = 1'b1;
      end
    endcase
  end

  lerp2 #(
      .WIDTH(WIDTH_CALC),
      .FBITS(FBITS),
      .X(WIDTH_CALC'((X - 1) << FBITS)),
      .Y(WIDTH_CALC'((Y - 1) << FBITS))
  ) lerp2_all (
      .p0({{(WIDTH_CALC - WIDTH_IN) {lerp2_p0[WIDTH_IN-1]}}, lerp2_p0}),
      .p1({{(WIDTH_CALC - WIDTH_IN) {lerp2_p1[WIDTH_IN-1]}}, lerp2_p1}),
      .p2({{(WIDTH_CALC - WIDTH_IN) {lerp2_p2[WIDTH_IN-1]}}, lerp2_p2}),
      .p3({{(WIDTH_CALC - WIDTH_IN) {lerp2_p3[WIDTH_IN-1]}}, lerp2_p3}),
      .x(WIDTH_CALC'(x << FBITS)),
      .y(WIDTH_CALC'(y << FBITS)),
      .val(lerp2_val),
      .start(lerp2_start),
      .done(lerp2_done),
      .error(lerp2_error),
      .clock,
      .reset(reset || lerp2_reset)
  );
endmodule
