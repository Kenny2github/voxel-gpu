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
    input logic reset_rasterize,
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
    DIV_LX,
    DIV_LY,
    DIV_LZ,
    DIV_HX,
    DIV_HY,
    DIV_HZ,
    DIVIDE,
    MEASURE,
    STORE_VOXEL,
    DONE_RASTERIZING,
    STORE_PIXEL,
    DONE_SHADING
  } state;
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

  logic div_reset, div_start, div_valid, div_done, dbz, ovf;
  logic [COORD_BITS+FRACT_BITS-1:0] div_a, div_b, div_val;

  div #(
      .WIDTH(COORD_BITS + FRACT_BITS),
      .FBITS(FRACT_BITS)
  ) div_all (
      .clk(clock),
      .rst(reset || div_reset),
      .start(div_start),
      .valid(div_valid),
      .busy(),
      .done(div_done),
      .dbz(dbz),
      .ovf(ovf),
      .a(div_a),
      .b(div_b),
      .val(div_val)
  );

  localparam PINF = (COORD_BITS + FRACT_BITS - 1)'('1);  // 0111...
  localparam MINF = ~PINF;  // 1000...

  always_ff @(posedge clock, posedge reset) begin
    if (reset) begin
      cycle_counter <= '0;
      _pixel <= '0;
      closest_voxel <= '0;
      closest_t <= PINF;
      t_min <= PINF;
      t_max <= MINF;
      tlx <= '0;
      tly <= '0;
      tlz <= '0;
      thx <= '0;
      thy <= '0;
      thz <= '0;
      state <= IDLE;
    end else begin
      case (state)
        IDLE: begin
          t_min <= PINF;
          t_max <= MINF;
          cycle_counter <= '0;
          if (do_rasterize) state <= DIV_LX;
          else if (do_shade) state <= STORE_PIXEL;
        end
        ERROR: begin
          t_min <= PINF;
          t_max <= MINF;
          cycle_counter <= '0;
          if (do_rasterize) state <= DIV_LX;
          else if (do_shade) state <= STORE_PIXEL;
        end
        DIV_LX: begin
          cycle_counter <= cycle_counter + 8'd1;
          if (div_valid && div_done) begin
            tlx   <= div_val;
            state <= DIV_LY;
          end else if (dbz || ovf) begin
            tlx   <= (lx + s) < s ? MINF : PINF;
            state <= DIV_LY;
          end
        end
        DIV_LY: begin
          cycle_counter <= cycle_counter + 8'd1;
          if (div_valid && div_done) begin
            tly   <= div_val;
            state <= DIV_LZ;
          end else if (dbz || ovf) begin
            tly   <= (ly + s) < s ? MINF : PINF;
            state <= DIV_LZ;
          end
        end
        DIV_LZ: begin
          cycle_counter <= cycle_counter + 8'd1;
          if (div_valid && div_done) begin
            tlz   <= div_val;
            state <= DIV_HX;
          end else if (dbz || ovf) begin
            tlz   <= (lz + s) < s ? MINF : PINF;
            state <= DIV_HX;
          end
        end
        DIV_HX: begin
          cycle_counter <= cycle_counter + 8'd1;
          if (div_valid && div_done) begin
            thx   <= div_val;
            state <= DIV_HY;
          end else if (dbz || ovf) begin
            thx   <= (hx + s) < s ? MINF : PINF;
            state <= DIV_HY;
          end
        end
        DIV_HY: begin
          cycle_counter <= cycle_counter + 8'd1;
          if (div_valid && div_done) begin
            thy   <= div_val;
            state <= DIV_HZ;
          end else if (dbz || ovf) begin
            thy   <= (hy + s) < s ? MINF : PINF;
            state <= DIV_HZ;
          end
        end
        DIV_HZ: begin
          cycle_counter <= cycle_counter + 8'd1;
          if (div_valid && div_done) begin
            thz   <= div_val;
            state <= DIVIDE;
          end else if (dbz || ovf) begin
            thz   <= (hz + s) < s ? MINF : PINF;
            state <= DIVIDE;
          end
        end
        DIVIDE: begin
          t_min <= (min_A_B_x + s) > (min_A_B_y + s) ? min_A_B_x : min_A_B_y;
          t_max <= (max_A_B_x + s) < (max_A_B_y + s) ? max_A_B_x : max_A_B_y;
          state <= MEASURE;
        end
        MEASURE: begin
          t_min <= (t_min + s) > (min_A_B_z + s) ? t_min : min_A_B_z;
          t_max <= (t_max + s) < (max_A_B_z + s) ? t_max : max_A_B_z;
          state <= STORE_VOXEL;
        end
        STORE_VOXEL: begin
          if ((t + s) > s && (t_min + s) <= (t_max + s) && (t + s) < (closest_t + s)) begin  // intersection!
            closest_t <= t;
            closest_voxel <= voxel_id;
          end
          state <= DONE_RASTERIZING;
        end
        DONE_RASTERIZING: begin
          if(reset_rasterize) begin
            state <= IDLE;
          end
        end
        STORE_PIXEL: begin
          if (voxel_id == closest_voxel) _pixel <= palette_entry;
          state <= DONE_SHADING;
        end
        DONE_SHADING: begin
          state <= IDLE;
        end
      endcase
    end
  end

  always_comb begin
    div_reset = 1'b0;
    div_start = 1'b0;
    div_a = '0;
    div_b = '0;
    rasterizing_done = 1'b0;
    shading_done = 1'b0;
    case (state)
      DIV_LX: begin
        div_reset = cycle_counter < 8'd2;
        div_start = cycle_counter < 8'd3;
        div_a = lx;
        div_b = cam_look_x;
      end
      DIV_LY: begin
        div_reset = cycle_counter < 8'd2;
        div_start = cycle_counter < 8'd3;
        div_a = ly;
        div_b = cam_look_y;
      end
      DIV_LZ: begin
        div_reset = cycle_counter < 8'd2;
        div_start = cycle_counter < 8'd3;
        div_a = lz;
        div_b = cam_look_z;
      end
      DIV_HX: begin
        div_reset = cycle_counter < 8'd2;
        div_start = cycle_counter < 8'd3;
        div_a = hx;
        div_b = cam_look_x;
      end
      DIV_HY: begin
        div_reset = cycle_counter < 8'd2;
        div_start = cycle_counter < 8'd3;
        div_a = hy;
        div_b = cam_look_y;
      end
      DIV_HZ: begin
        div_reset = cycle_counter < 8'd2;
        div_start = cycle_counter < 8'd3;
        div_a = hz;
        div_b = cam_look_z;
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
