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
  enum logic [3:0] {
    IDLE,
    RESET_X,
    START_X,
    DIV_X,
    RESET_Y,
    START_Y,
    DIV_Y,
    RESET_XY,
    START_XY,
    DIV_XY,
    SUM,
    ERROR
  } state;

  logic signed [WIDTH-1:0] a, b, div_val;
  logic div_start, div_reset, div_valid, div_done, dbz, ovf;

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
          state <= START_X;
        end
        START_X: begin
          state <= DIV_X;
        end
        DIV_X: begin
          if (div_valid && div_done) begin
            state <= RESET_Y;
            val   <= val + div_val;
          end else if (dbz || ovf) state <= ERROR;
        end
        RESET_Y: begin
          state <= START_Y;
        end
        START_Y: begin
          state <= DIV_Y;
        end
        DIV_Y: begin
          if (div_valid && div_done) begin
            state <= RESET_XY;
            val   <= val + div_val;
          end else if (dbz || ovf) state <= ERROR;
        end
        RESET_XY: begin
          state <= START_XY;
        end
        START_XY: begin
          state <= DIV_XY;
        end
        DIV_XY: begin
          if (div_valid && div_done) begin
            state <= SUM;
            val   <= val + div_val;
          end else if (dbz || ovf) state <= ERROR;
        end
        SUM: begin
          state <= IDLE;
        end
      endcase
    end
  end

  always_comb begin
    a = '0;
    b = '0;
    div_start = 1'b0;
    div_reset = 1'b0;
    done = 1'b0;
    error = 1'b0;
    case (state)
      RESET_X: begin
        div_reset = 1'b1;
        a = WIDTH'(((p1 - p0) * {{WIDTH{x[WIDTH-1]}}, x}) >>> FBITS);
        b = X;
      end
      START_X: begin
        div_start = 1'b1;
        a = WIDTH'(((p1 - p0) * {{WIDTH{x[WIDTH-1]}}, x}) >>> FBITS);
        b = X;
      end
      DIV_X: begin
        a = WIDTH'(((p1 - p0) * {{WIDTH{x[WIDTH-1]}}, x}) >>> FBITS);
        b = X;
      end
      RESET_Y: begin
        div_reset = 1'b1;
        a = WIDTH'(((p2 - p0) * {{WIDTH{y[WIDTH-1]}}, y}) >>> FBITS);
        b = Y;
      end
      START_Y: begin
        div_start = 1'b1;
        a = WIDTH'(((p2 - p0) * {{WIDTH{y[WIDTH-1]}}, y}) >>> FBITS);
        b = Y;
      end
      DIV_Y: begin
        a = WIDTH'(((p2 - p0) * {{WIDTH{y[WIDTH-1]}}, y}) >>> FBITS);
        b = Y;
      end
      RESET_XY: begin
        div_reset = 1'b1;
        a = WIDTH'(((p0 - p1 + p3 - p2) * {{WIDTH{x[WIDTH-1]}}, x} * y) >>> (2 * FBITS));
        b = WIDTH'(({{WIDTH{X[WIDTH-1]}}, X} * Y) >> FBITS);
      end
      START_XY: begin
        div_start = 1'b1;
        a = WIDTH'(((p0 - p1 + p3 - p2) * {{WIDTH{x[WIDTH-1]}}, x} * y) >>> (2 * FBITS));
        b = WIDTH'(({{WIDTH{X[WIDTH-1]}}, X} * Y) >> FBITS);
      end
      DIV_XY: begin
        a = WIDTH'(((p0 - p1 + p3 - p2) * {{WIDTH{x[WIDTH-1]}}, x} * y) >>> (2 * FBITS));
        b = WIDTH'(({{WIDTH{X[WIDTH-1]}}, X} * Y) >> FBITS);
      end
      SUM: begin
        done = 1'b1;
      end
      ERROR: begin
        error = 1'b1;
      end
    endcase
  end

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
      .ovf(ovf),
      .a(a),
      .b(b),
      .val(div_val)
  );

endmodule
