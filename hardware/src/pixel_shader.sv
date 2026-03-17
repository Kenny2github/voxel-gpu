module pixel_shader #(
    parameter INDEX = 0,
    parameter INDEX_BITS = 32,
    parameter COORD_BITS = 10,
    parameter PALETTE_BITS = 32 - (COORD_BITS * 3),
    parameter FRACT_BITS = 10,
    parameter PIXEL_BITS = 8
) (
    input logic do_rasterize,
    input logic do_shade,
    input logic signed [COORD_BITS-1:0] voxel_x,
    input logic signed [COORD_BITS-1:0] voxel_y,
    input logic signed [COORD_BITS-1:0] voxel_z,
    input logic [PALETTE_BITS-1:0] voxel_id,
    input logic [PIXEL_BITS-1:0] palette_entry,
    input logic signed [COORD_BITS+FRACT_BITS-1:0] cam_pos_x,
    input logic signed [COORD_BITS+FRACT_BITS-1:0] cam_pos_y,
    input logic signed [COORD_BITS+FRACT_BITS-1:0] cam_pos_z,
    input logic signed [COORD_BITS+FRACT_BITS-1:0] cam_look_x,
    input logic signed [COORD_BITS+FRACT_BITS-1:0] cam_look_y,
    input logic signed [COORD_BITS+FRACT_BITS-1:0] cam_look_z,
    input logic [INDEX_BITS-1:0] pixel_index,
    output logic rasterizing_done,
    output logic shading_done,
    output logic error,
    output wire [PIXEL_BITS-1:0] pixel,
    input logic reset,
    input logic clock
);
  enum logic [3:0] {
    IDLE,
    ERROR,
    DIVIDE,
    MEASURE,
    STORE_VOXEL,
    DONE_RASTERIZING,
    STORE_PIXEL,
    DONE_SHADING
  }
      state, next_state;
  assign error = (state == ERROR);

  logic [PIXEL_BITS-1:0] _pixel;
  assign pixel = (pixel_index == INDEX) ? _pixel : 'z;

  logic [PALETTE_BITS-1:0] closest_voxel;

  logic [7:0] cycle_counter;

  logic signed [COORD_BITS+FRACT_BITS-1:0]
      lx, ly, lz, hx, hy, hz, tlx, tly, tlz, thx, thy, thz, closest_t, t_min, t_max,
      min_A_B_x, min_A_B_y, min_A_B_z, max_A_B_x, max_A_B_y, max_A_B_z, t, s;
  // toggle sign bit when comparing (i.e. shift into unsigned range) by adding s
  // assign s = (1 << (COORD_BITS + FRACT_BITS - 1));
  assign s = 0;
  
  assign lx = {voxel_x, FRACT_BITS'(0)} - cam_pos_x;
  assign ly = {voxel_y, FRACT_BITS'(0)} - cam_pos_y;
  assign lz = {voxel_z, FRACT_BITS'(0)} - cam_pos_z;
  assign hx = {voxel_x + 1'b1, FRACT_BITS'(0)} - cam_pos_x;
  assign hy = {voxel_y + 1'b1, FRACT_BITS'(0)} - cam_pos_y;
  assign hz = {voxel_z + 1'b1, FRACT_BITS'(0)} - cam_pos_z;

  assign min_A_B_x = (tlx + s) < (thx + s) ? tlx : thx;
  assign min_A_B_y = (tly + s) < (thy + s) ? tly : thy;
  assign min_A_B_z = (tlz + s) < (thz + s) ? tlz : thz;
  assign max_A_B_x = (tlx + s) > (thx + s) ? tlx : thx;
  assign max_A_B_y = (tly + s) > (thy + s) ? tly : thy;
  assign max_A_B_z = (tlz + s) > (thz + s) ? tlz : thz;

  assign t = (t_min + s) > s ? t_min : t_max;

  logic div_start;
  logic [0:5] div_valid;
  logic [0:5] div_error;
  logic [0:5] done;
  logic [0:5] valid;
  logic [0:5] dbz;
  assign div_valid = dbz | (valid & done);

  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divlx (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(valid[0]),
      .busy(),
      .done(done[0]),
      .dbz(dbz[0]),
      .ovf(div_error[0]),
      .a(lx),
      .b(cam_look_x),
      .val(tlx)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divly (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(valid[1]),
      .busy(),
      .done(done[1]),
      .dbz(dbz[1]),
      .ovf(div_error[1]),
      .a(ly),
      .b(cam_look_y),
      .val(tly)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divlz (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(valid[2]),
      .busy(),
      .done(done[2]),
      .dbz(dbz[2]),
      .ovf(div_error[2]),
      .a(lz),
      .b(cam_look_z),
      .val(tlz)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divhx (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(valid[3]),
      .busy(),
      .done(done[3]),
      .dbz(dbz[3]),
      .ovf(div_error[3]),
      .a(hx),
      .b(cam_look_x),
      .val(thx)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divhy (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(valid[4]),
      .busy(),
      .done(done[4]),
      .dbz(dbz[4]),
      .ovf(div_error[4]),
      .a(hy),
      .b(cam_look_y),
      .val(thy)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divhz (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(valid[5]),
      .busy(),
      .done(done[5]),
      .dbz(dbz[5]),
      .ovf(div_error[5]),
      .a(hz),
      .b(cam_look_z),
      .val(thz)
  );

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
        if (do_rasterize) next_state = DIVIDE;
        else if (do_shade) next_state = STORE_PIXEL;
        else next_state = IDLE;
      end
      ERROR: begin
        if (do_rasterize) next_state = DIVIDE;
        else if (do_shade) next_state = STORE_PIXEL;
        else next_state = ERROR;
      end
      DIVIDE: begin
        if (div_error) next_state = ERROR;
        else if (&div_valid) next_state = MEASURE;
        else next_state = DIVIDE;
      end
      MEASURE: begin
        next_state = STORE_VOXEL;
      end
      STORE_VOXEL: begin
        next_state = DONE_RASTERIZING;
      end
      DONE_RASTERIZING: begin
        next_state = IDLE;
      end
      STORE_PIXEL: begin
        next_state = DONE_SHADING;
      end
      DONE_SHADING: begin
        next_state = IDLE;
      end
    endcase
  end

  localparam PINF = (COORD_BITS + FRACT_BITS - 1)'('1); // 0111...
  localparam MINF = ~PINF; // 1000...

  always_ff @(posedge clock, posedge reset) begin
    if (reset) begin
      cycle_counter <= '0;
      _pixel <= '0;
      closest_voxel <= '0;
      closest_t <= PINF;
      t_min <= PINF;
      t_max <= MINF;
    end else begin
      case (state)
        IDLE: begin
          t_min <= PINF;
          t_max <= MINF;
          cycle_counter <= '0;
        end
        ERROR: begin
          t_min <= PINF;
          t_max <= MINF;
          cycle_counter <= '0;
        end
        DIVIDE: begin
          cycle_counter <= cycle_counter + 8'd1;
          if (dbz[0] | dbz[3]) begin
            if ((lx + s) < s) begin  // max(min_A_B_x=-inf, min_A_B_y) = min_A_B_y
              if (dbz[1] | dbz[4]) begin
                t_min <= (ly + s) < s ? MINF : PINF;
              end else begin
                t_min <= min_A_B_y;
              end
            end else begin // max(min_A_B_x=+inf, min_A_B_y) = +inf
              t_min <= PINF;
            end
            if ((hx + s) < s) begin // min(max_A_B_x=-inf, max_A_B_y) = -inf
              t_max <= MINF;
            end else begin // min(max_A_B_x=+inf, max_A_B_y) = max_A_B_y
              if (dbz[1] | dbz[4]) begin
                t_max <= (hy + s) < s ? MINF : PINF;
              end else begin
                t_max <= max_A_B_y;
              end
            end
          end else if (dbz[1] | dbz[4]) begin
            // max(min_A_B_x!=inf, min_A_B_y=-inf) = min_A_B_x
            // max(min_A_B_x!=inf, min_A_B_y=+inf) = +inf
            t_min <= (ly + s) < s ? min_A_B_x : PINF;
            // min(max_A_B_x!=inf, max_A_B_y=-inf) = -inf
            // min(max_A_B_x!=inf, max_A_B_y=+inf) = max_A_B_x
            t_max <= (hy + s) < s ? MINF : max_A_B_x;
          end else begin
            t_min <= (min_A_B_x + s) > (min_A_B_y + s) ? min_A_B_x : min_A_B_y;
            t_max <= (max_A_B_x + s) < (max_A_B_y + s) ? max_A_B_x : max_A_B_y;
          end
        end
        MEASURE: begin
          if (dbz[2] | dbz[5]) begin
            // max(t_min, min_A_B_z=-inf) = t_min
            // max(t_min, min_A_B_z=+inf) = +inf
            t_min <= (lz + s) < s ? t_min : PINF;
            // min(t_max, max_A_B_z=-inf) = -inf
            // min(t_max, max_A_B_z=+inf) = t_max
            t_max <= (hz + s) < s ? MINF : t_max;
          end else begin
            t_min <= (t_min + s) > (min_A_B_z + s) ? t_min : min_A_B_z;
            t_max <= (t_max + s) < (max_A_B_z + s) ? t_max : max_A_B_z;
          end
        end
        STORE_VOXEL: begin
          if ((t + s) > s && (t_min + s) <= (t_max + s) && (t + s) < (closest_t + s)) begin  // intersection!
            closest_t <= t;
            closest_voxel <= voxel_id;
          end
        end
        STORE_PIXEL: begin
          if (voxel_id == closest_voxel) _pixel <= palette_entry;
        end
      endcase
    end
  end

  always_comb begin
    div_start = 1'b0;
    rasterizing_done = 1'b0;
    shading_done = 1'b0;
    case (state)
      DIVIDE: begin
        div_start = cycle_counter < 8'd2;
      end
      DONE_RASTERIZING: begin
        rasterizing_done = 1'b1;
      end
      DONE_SHADING: begin
        shading_done = 1'b1;
      end
    endcase
  end

endmodule
