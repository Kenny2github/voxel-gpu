module lerp2 #(
    parameter WIDTH = 32,
    parameter FBITS = 16,
    parameter X = 320,
    parameter Y = 240
) (
    input logic signed [WIDTH-1:0] p0,
    input logic signed [WIDTH-1:0] p1,
    input logic signed [WIDTH-1:0] p2,
    input logic signed [WIDTH-1:0] p3,
    input logic signed [WIDTH-1:0] x,
    input logic signed [WIDTH-1:0] y,
    output logic signed [WIDTH-1:0] val,
    input logic start,
    output logic done,
    output logic error,
    input logic clock,
    input logic reset
);
  enum logic [4:0] {
    IDLE,
    RESET_X,
    START_MUL_X,
    MUL_X,
    START_DIV_X,
    DIV_X,
    RESET_Y,
    START_MUL_Y,
    MUL_Y,
    START_DIV_Y,
    DIV_Y,
    RESET_XY,
    START_MUL_XY,
    MUL_XY,
    START_DIV_XY,
    DIV_XY,
    SUM,
    ERROR
  } state;

  logic signed [WIDTH-1:0] div_a, div_b, div_val;
  logic div_start, div_reset, div_valid, div_done, dbz, div_ovf;
  logic signed [WIDTH-1:0] mul_a, mul_b, mul_val, mul_x_a, mul_y_a, mul_xy_a, mul_xy_b;
  logic mul_start, mul_reset, mul_valid, mul_done, mul_ovf;

  always_ff @(posedge clock or posedge reset) begin
    if (reset) begin
      state <= IDLE;
      val   <= '0;
    end else begin
      case (state)
        IDLE: begin
          if (start) begin
            state <= RESET_X;
            val   <= p0;
          end
        end
        RESET_X: begin
          state <= START_MUL_X;
        end
        START_MUL_X: begin
          state <= MUL_X;
        end
        MUL_X: begin
          if (mul_valid && mul_done) state <= START_DIV_X;
          else if (mul_ovf) state <= ERROR;
        end
        START_DIV_X: begin
          state <= DIV_X;
        end
        DIV_X: begin
          if (div_valid && div_done) begin
            state <= RESET_Y;
            val   <= val + div_val;
          end else if (dbz || div_ovf) state <= ERROR;
        end
        RESET_Y: begin
          state <= START_MUL_Y;
        end
        START_MUL_Y: begin
          state <= MUL_Y;
        end
        MUL_Y: begin
          if (mul_valid && mul_done) state <= START_DIV_Y;
          else if (mul_ovf) state <= ERROR;
        end
        START_DIV_Y: begin
          state <= DIV_Y;
        end
        DIV_Y: begin
          if (div_valid && div_done) begin
            state <= RESET_XY;
            val   <= val + div_val;
          end else if (dbz || div_ovf) state <= ERROR;
        end
        RESET_XY: begin
          state <= START_MUL_XY;
        end
        START_MUL_XY: begin
          state <= MUL_XY;
        end
        MUL_XY: begin
          if (mul_valid && mul_done) state <= START_DIV_XY;
          else if (mul_ovf) state <= ERROR;
        end
        START_DIV_XY: begin
          state <= DIV_XY;
        end
        DIV_XY: begin
          if (div_valid && div_done) begin
            state <= SUM;
            val   <= val + div_val;
          end else if (dbz || div_ovf) state <= ERROR;
        end
        SUM: begin
          state <= IDLE;
        end
      endcase
    end
  end

  always_comb begin
    mul_a = '0;
    mul_b = '0;
    mul_start = 1'b0;
    mul_reset = 1'b0;
    mul_x_a = WIDTH'(p1 - p0);
    mul_y_a = WIDTH'(p2 - p0);
    mul_xy_a = WIDTH'(p0 - p1 + p3 - p2);
    mul_xy_b = WIDTH'(({{WIDTH{x[WIDTH-1]}}, x} * y) >>> FBITS);
    div_a = mul_val;
    div_b = '0;
    div_start = 1'b0;
    div_reset = 1'b0;
    done = 1'b0;
    error = 1'b0;
    case (state)
      RESET_X: begin
        mul_reset = 1'b1;
        div_reset = 1'b1;
      end
      START_MUL_X: begin
        mul_start = 1'b1;
        mul_a = mul_x_a;
        mul_b = x;
      end
      MUL_X: begin
        mul_a = mul_x_a;
        mul_b = x;
      end
      START_DIV_X: begin
        div_start = 1'b1;
        div_b = X;
      end
      DIV_X: begin
        div_b = X;
      end
      RESET_Y: begin
        mul_reset = 1'b1;
        div_reset = 1'b1;
      end
      START_MUL_Y: begin
        mul_start = 1'b1;
        mul_a = mul_y_a;
        mul_b = y;
      end
      MUL_Y: begin
        mul_a = mul_y_a;
        mul_b = y;
      end
      START_DIV_Y: begin
        div_start = 1'b1;
        div_b = Y;
      end
      DIV_Y: begin
        div_b = Y;
      end
      RESET_XY: begin
        mul_reset = 1'b1;
        div_reset = 1'b1;
      end
      START_MUL_XY: begin
        mul_start = 1'b1;
        mul_a = mul_xy_a;
        mul_b = mul_xy_b;
      end
      MUL_XY: begin
        mul_a = mul_xy_a;
        mul_b = mul_xy_b;
      end
      START_DIV_XY: begin
        div_start = 1'b1;
        div_b = WIDTH'(({{WIDTH{X[WIDTH-1]}}, X} * Y) >> FBITS);
      end
      DIV_XY: begin
        div_b = WIDTH'(({{WIDTH{X[WIDTH-1]}}, X} * Y) >> FBITS);
      end
      SUM: begin
        done = 1'b1;
      end
      ERROR: begin
        error = 1'b1;
      end
    endcase
  end

  mul #(
      .WIDTH(WIDTH),
      .FBITS(FBITS)
  ) mul_all (
      .clk(clock),
      .rst(reset || mul_reset),
      .start(mul_start),
      .busy(),
      .done(mul_done),
      .valid(mul_valid),
      .ovf(mul_ovf),
      .a(mul_a),
      .b(mul_b),
      .val(mul_val)
  );

  div #(
      .WIDTH(WIDTH),
      .FBITS(FBITS)
  ) div_all (
      .clk(clock),
      .rst(reset || div_reset),
      .start(div_start),
      .valid(div_valid),
      .busy(),
      .done(div_done),
      .dbz(dbz),
      .ovf(div_ovf),
      .a(div_a),
      .b(div_b),
      .val(div_val)
  );

endmodule
