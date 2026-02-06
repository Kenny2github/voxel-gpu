module pixel_shader #(
    parameter ROW = 0,
    parameter COL = 0,
    parameter ROW_BITS = 8,
    parameter COL_BITS = 8,
    parameter COORD_BITS = 8,
    parameter PALETTE_BITS = 8,
    parameter FRAC_BITS = 8,
    parameter PIXEL_BITS = 8
) (
    input logic do_rasterize,
    input logic do_shade,
    input logic [COORD_BITS-1:0] voxel_x,
    input logic [COORD_BITS-1:0] voxel_y,
    input logic [COORD_BITS-1:0] voxel_z,
    input logic [PALETTE_BITS-1:0] voxel_id,
    input logic [PIXEL_BITS-1:0] palette_entry,
    input logic signed [COORD_BITS+FRAC_BITS-1:0] cam_pos_x,
    input logic signed [COORD_BITS+FRAC_BITS-1:0] cam_pos_y,
    input logic signed [COORD_BITS+FRAC_BITS-1:0] cam_pos_z,
    input logic signed [COORD_BITS+FRAC_BITS-1:0] cam_look_x,
    input logic signed [COORD_BITS+FRAC_BITS-1:0] cam_look_y,
    input logic signed [COORD_BITS+FRAC_BITS-1:0] cam_look_z,
    input logic [ROW_BITS-1:0] row,
    input logic [COL_BITS-1:0] col,
    output logic rasterizing_done,
    output logic shading_done,
    output wire [PIXEL_BITS-1:0] pixel,
    input logic reset,
    input logic clock
);
  enum logic [3:0] {
    IDLE,
    MEASURE,
    STORE_VOXEL,
    DONE_RASTERIZING,
    STORE_PIXEL,
    DONE_SHADING
  }
      state, next_state;

  logic [PIXEL_BITS-1:0] _pixel;
  assign pixel = (row == ROW && col == COL) ? _pixel : 'z;

  logic [PALETTE_BITS-1:0] closest_voxel;

  logic [7:0] cycle_counter;

  logic signed [COORD_BITS+FRAC_BITS-1:0] AOx, AOy, AOz, BOx, BOy, BOz, tAx, tAy, tAz, tBx, tBy, tBz, closest_t, t_min, t_max;

  assign AOx = {voxel_x, FRAC_BITS'(0)} - cam_pos_x;
  assign AOy = {voxel_y, FRAC_BITS'(0)} - cam_pos_y;
  assign AOz = {voxel_z, FRAC_BITS'(0)} - cam_pos_z;
  assign BOx = {voxel_x + 1'b1, FRAC_BITS'(0)} - cam_pos_x;
  assign BOy = {voxel_y + 1'b1, FRAC_BITS'(0)} - cam_pos_y;
  assign BOz = {voxel_z + 1'b1, FRAC_BITS'(0)} - cam_pos_z;

  assign tAx = {AOx, FRAC_BITS'(0)} / (cam_look_x || 1);
  assign tAy = {AOy, FRAC_BITS'(0)} / (cam_look_y || 1);
  assign tAz = {AOz, FRAC_BITS'(0)} / (cam_look_z || 1);
  assign tBx = {BOx, FRAC_BITS'(0)} / (cam_look_x || 1);
  assign tBy = {BOy, FRAC_BITS'(0)} / (cam_look_y || 1);
  assign tBz = {BOz, FRAC_BITS'(0)} / (cam_look_z || 1);

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
        if (do_rasterize) next_state = MEASURE;
        else if (do_shade) next_state = STORE_PIXEL;
        else next_state = IDLE;
      end
      MEASURE: begin
        if (cycle_counter == 8'd7) next_state = STORE_VOXEL;
        else next_state = MEASURE;
      end
      STORE_VOXEL: begin
        next_state = DONE_RASTERIZING;
      end
      DONE_RASTERIZING: begin
        if (do_rasterize) next_state = MEASURE;
        else next_state = IDLE;
      end
      STORE_PIXEL: begin
        next_state = DONE_SHADING;
      end
      DONE_SHADING: begin
        if (do_shade) next_state = STORE_PIXEL;
        else next_state = IDLE;
      end
    endcase
  end

  always_ff @(posedge clock, posedge reset) begin
    if (reset) begin
      cycle_counter <= '0;
      _pixel <= '0;
      closest_voxel <= '0;
      closest_t <= (COORD_BITS+FRAC_BITS-1)'('1);
      t_min <= (COORD_BITS+FRAC_BITS-1)'('1);
      t_max <= ~(COORD_BITS+FRAC_BITS-1)'('1);
    end else begin
      case (state)
        IDLE: begin
          closest_t <= (COORD_BITS+FRAC_BITS-1)'('1);
          t_min <= (COORD_BITS+FRAC_BITS-1)'('1);
          t_max <= ~(COORD_BITS+FRAC_BITS-1)'('1);
          cycle_counter <= '0;
        end
        MEASURE: begin
          cycle_counter <= cycle_counter + 1'b1;
          if (cycle_counter == 8'd6) begin
            t_min <= ((AOx < BOx) ? tAx : tBx) > ((AOy < BOy) ? tAy : tBy) ? ((AOx < BOx) ? tAx : tBx) : ((AOy < BOy) ? tAy : tBy);
            t_max <= ((AOx < BOx) ? tAx : tBx) < ((AOy < BOy) ? tAy : tBy) ? ((AOx < BOx) ? tAx : tBx) : ((AOy < BOy) ? tAy : tBy);
          end else if (cycle_counter == 8'd7) begin
            t_min <= t_min > ((AOz < BOz) ? tAz : tBz) ? t_min : ((AOz < BOz) ? tAz : tBz);
            t_max <= t_max < ((AOz < BOz) ? tAz : tBz) ? t_max : ((AOz < BOz) ? tAz : tBz);
          end
        end
        STORE_VOXEL: begin
          if (t_min < t_max && t_min < closest_t) begin  // intersection!
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
    rasterizing_done = 1'b0;
    shading_done = 1'b0;
    case (state)
      DONE_RASTERIZING: begin
        rasterizing_done = 1'b1;
      end
      DONE_SHADING: begin
        shading_done = 1'b1;
      end
    endcase
  end

endmodule
