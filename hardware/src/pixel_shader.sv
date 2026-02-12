module pixel_shader #(
    parameter INDEX = 0,
    parameter INDEX_BITS = 32,
    parameter COORD_BITS = 8,
    parameter PALETTE_BITS = 32 - (COORD_BITS * 3),
    parameter FRACT_BITS = 8,
    parameter PIXEL_BITS = 8
) (
    input logic do_rasterize,
    input logic do_shade,
    input logic [COORD_BITS-1:0] voxel_x,
    input logic [COORD_BITS-1:0] voxel_y,
    input logic [COORD_BITS-1:0] voxel_z,
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
      AOx, AOy, AOz, BOx, BOy, BOz, tAx, tAy, tAz, tBx, tBy, tBz, closest_t, t_min, t_max,
      min_A_B_x, min_A_B_y, min_A_B_z, s;
  logic A_lt_B_x, A_lt_B_y, A_lt_B_z;
  // toggle sign bit when comparing (i.e. shift into unsigned range) by adding s
  assign s   = (1 << (COORD_BITS + FRACT_BITS - 1));

  assign AOx = {voxel_x, FRACT_BITS'(0)} - cam_pos_x;
  assign AOy = {voxel_y, FRACT_BITS'(0)} - cam_pos_y;
  assign AOz = {voxel_z, FRACT_BITS'(0)} - cam_pos_z;
  assign BOx = {voxel_x + 1'b1, FRACT_BITS'(0)} - cam_pos_x;
  assign BOy = {voxel_y + 1'b1, FRACT_BITS'(0)} - cam_pos_y;
  assign BOz = {voxel_z + 1'b1, FRACT_BITS'(0)} - cam_pos_z;

  assign A_lt_B_x = (AOx + s) < (BOx + s);
  assign A_lt_B_y = (AOy + s) < (BOy + s);
  assign A_lt_B_z = (AOz + s) < (BOz + s);
  assign min_A_B_x = A_lt_B_x ? tAx : tBx;
  assign min_A_B_y = A_lt_B_y ? tAy : tBy;
  assign min_A_B_z = A_lt_B_z ? tAz : tBz;

  logic div_start;
  wand  div_valid;
  wor   div_error;

  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divAx (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(div_valid),
      .busy(),
      .done(div_valid),
      .dbz(div_error),
      .ovf(div_error),
      .a(AOx),
      .b(cam_look_x ? cam_look_x : (COORD_BITS + FRACT_BITS)'(1 << FRACT_BITS)),
      .val(tAx)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divAy (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(div_valid),
      .busy(),
      .done(div_valid),
      .dbz(div_error),
      .ovf(div_error),
      .a(AOy),
      .b(cam_look_y ? cam_look_y : (COORD_BITS + FRACT_BITS)'(1 << FRACT_BITS)),
      .val(tAy)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divAz (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(div_valid),
      .busy(),
      .done(div_valid),
      .dbz(div_error),
      .ovf(div_error),
      .a(AOz),
      .b(cam_look_z ? cam_look_z : (COORD_BITS + FRACT_BITS)'(1 << FRACT_BITS)),
      .val(tAz)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divBx (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(div_valid),
      .busy(),
      .done(div_valid),
      .dbz(div_error),
      .ovf(div_error),
      .a(BOx),
      .b(cam_look_x ? cam_look_x : (COORD_BITS + FRACT_BITS)'(1 << FRACT_BITS)),
      .val(tBx)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divBy (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(div_valid),
      .busy(),
      .done(div_valid),
      .dbz(div_error),
      .ovf(div_error),
      .a(BOy),
      .b(cam_look_y ? cam_look_y : (COORD_BITS + FRACT_BITS)'(1 << FRACT_BITS)),
      .val(tBy)
  );
  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) divBz (
      .clk(clock),
      .rst(reset),
      .start(div_start),
      .valid(div_valid),
      .busy(),
      .done(div_valid),
      .dbz(div_error),
      .ovf(div_error),
      .a(BOz),
      .b(cam_look_z ? cam_look_z : (COORD_BITS + FRACT_BITS)'(1 << FRACT_BITS)),
      .val(tBz)
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
        else if (div_valid) next_state = MEASURE;
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

  always_ff @(posedge clock, posedge reset) begin
    if (reset) begin
      cycle_counter <= '0;
      _pixel <= '0;
      closest_voxel <= '0;
      closest_t <= (COORD_BITS + FRACT_BITS - 1)'('1);
      t_min <= (COORD_BITS + FRACT_BITS - 1)'('1);
      t_max <= ~(COORD_BITS + FRACT_BITS - 1)'('1);
    end else begin
      case (state)
        IDLE: begin
          t_min <= (COORD_BITS + FRACT_BITS - 1)'('1);
          t_max <= ~(COORD_BITS + FRACT_BITS - 1)'('1);
          cycle_counter <= '0;
        end
        ERROR: begin
          t_min <= (COORD_BITS + FRACT_BITS - 1)'('1);
          t_max <= ~(COORD_BITS + FRACT_BITS - 1)'('1);
          cycle_counter <= '0;
        end
        DIVIDE: begin
          cycle_counter <= cycle_counter + 8'd1;
          t_min <= (min_A_B_x + s) > (min_A_B_y + s) ? min_A_B_x : min_A_B_y;
          t_max <= (min_A_B_x + s) < (min_A_B_y + s) ? min_A_B_x : min_A_B_y;
        end
        MEASURE: begin
          t_min <= (t_min + s) > (min_A_B_z + s) ? t_min : min_A_B_z;
          t_max <= (t_max + s) < (min_A_B_z + s) ? t_max : min_A_B_z;
        end
        STORE_VOXEL: begin
          if ((t_min + s) <= (t_max + s) && (t_min + s) < (closest_t + s)) begin  // intersection!
            closest_t <= t_min;
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
