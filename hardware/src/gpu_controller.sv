`include "common.svh"

module gpu_controller #(
    parameter MY_ROWS = 0,
    parameter MY_COLS = 0,
    parameter TOTAL_ROWS = 0,
    parameter TOTAL_COLS = 0,
    parameter ROW_BITS = $clog2(TOTAL_ROWS),
    parameter COL_BITS = $clog2(TOTAL_COLS),
    parameter COORD_BITS = 8,
    parameter PALETTE_BITS = 8,
    parameter FRAC_BITS = 8,
    parameter PIXEL_BITS = 8
) (
    input  camera                  cam,
    output logic  [          31:0] m1_address,        //    m1.address
    output logic  [           7:0] m1_writedata,      //      .writedata
    output logic                   m1_write,          //      .write
    input  logic                   m1_waitrequest,    //      .waitrequest
    input  logic  [           7:0] m1_readdata,       //      .readdata
    output logic                   m1_read,           //      .read
    input  logic                   m1_readdatavalid,  //      .readdatavalid
    input  logic  [PIXEL_BITS-1:0] start_row,
    input  logic  [PIXEL_BITS-1:0] start_col,
    input  logic  [          31:0] pixel_buffer,
    input  logic  [          31:0] voxel_buffer,
    input  logic  [          31:0] voxel_count,
    input  logic  [          31:0] palette_buffer,
    input  logic  [          31:0] palette_length,
    input  logic                   do_render,
    input  logic                   clear_interrupt,
    output logic                   irq,
    input  logic                   reset,
    input  logic                   clock
);
  enum logic [3:0] {
    IDLE,
    RASTERIZING,
    RASTERIZE,
    FETCH_VOXEL,
    SHADING,
    SHADE,
    FETCH_ENTRY,
    WRITEOUT,
    WRITE,
    FETCH_PIXEL,
    INTERRUPT
  }
      state, next_state;

  logic [COORD_BITS-1:0] voxel_x, voxel_y, voxel_z;
  logic [PALETTE_BITS-1:0] voxel_id;
  logic [COORD_BITS * 3 - 1:0] voxel_num;
  logic [PIXEL_BITS-1:0] palette_entry;
  logic [PALETTE_BITS-1:0] entry_num;
  logic [ROW_BITS-1:0] row;
  logic [COL_BITS-1:0] col;
  wire [PIXEL_BITS-1:0] pixel;
  logic [ROW_BITS+COL_BITS-1:0] pixel_num;
  logic [7:0] cycle_counter;
  logic read_valid, do_rasterize, rasterizing_done, do_shade, shading_done, writing_done;

  // bilinear interpolation...
  function logic [COORD_BITS+FRAC_BITS-1:0] lerp2;
    input logic [COORD_BITS+FRAC_BITS-1:0] p0, p1, p2, p3, x, y, X, Y;
    output lerp2;
    begin
      lerp2 = p0 + (p1 - p0) * x / X + (p2 - p0) * y / Y + (p0 - p1 + p3 - p2) * x * y / (X * Y);
    end
  endfunction

  genvar r, c;
  generate
    for (r = 0; r < MY_ROWS; ++r) begin
      for (c = 0; c < MY_COLS; ++c) begin
        pixel_shader #(
            .ROW(r),
            .COL(c),
            .ROW_BITS(ROW_BITS),
            .COL_BITS(COL_BITS),
            .COORD_BITS(COORD_BITS),
            .PALETTE_BITS(PALETTE_BITS),
            .FRAC_BITS(FRAC_BITS),
            .PIXEL_BITS(PIXEL_BITS)
        ) shader (
            .cam_pos_x(cam.pos.x),
            .cam_pos_y(cam.pos.y),
            .cam_pos_z(cam.pos.z),
            .cam_look_x(lerp2(
                cam.look0.x,
                cam.look1.x,
                cam.look2.x,
                cam.look3.x,
                c + start_col,
                r + start_row,
                TOTAL_COLS,
                TOTAL_ROWS
            )),
            .cam_look_y(lerp2(
                cam.look0.y,
                cam.look1.y,
                cam.look2.y,
                cam.look3.y,
                c + start_col,
                r + start_row,
                TOTAL_COLS,
                TOTAL_ROWS
            )),
            .cam_look_z(lerp2(
                cam.look0.z,
                cam.look1.z,
                cam.look2.z,
                cam.look3.z,
                c + start_col,
                r + start_row,
                TOTAL_COLS,
                TOTAL_ROWS
            )),
            .*
        );
      end
    end
  endgenerate

  always_ff @(posedge clock, posedge reset) begin
    if (reset) begin
      state <= IDLE;
    end else begin
      state <= next_state;
    end
  end

  always_comb begin
    next_state = IDLE;
    case (state)
      IDLE: begin
        if (do_render) next_state = RASTERIZING;
        else next_state = IDLE;
      end
      RASTERIZING: begin
        next_state = FETCH_VOXEL;
      end
      FETCH_VOXEL: begin
        if (read_valid) next_state = RASTERIZE;
        else next_state = FETCH_VOXEL;
      end
      RASTERIZE: begin
        if (rasterizing_done) begin
          if (voxel_num >= voxel_count) next_state = SHADING;
          else next_state = FETCH_VOXEL;
        end else next_state = RASTERIZE;
      end
      SHADING: begin
        next_state = FETCH_ENTRY;
      end
      FETCH_ENTRY: begin
        if (read_valid) next_state = SHADE;
        else next_state = FETCH_ENTRY;
      end
      SHADE: begin
        if (shading_done) begin
          if (entry_num >= entry_count) next_state = WRITEOUT;
          else next_state = FETCH_ENTRY;
        end else next_state = SHADE;
      end
      WRITEOUT: begin
        next_state = WRITE;
      end
      FETCH_PIXEL: begin
        next_state = WRITE;
      end
      WRITE: begin
        if (writing_done) begin
          if (pixel_num >= pixel_count) next_state = INTERRUPT;
          else next_state = FETCH_PIXEL;
        end else next_state = WRITE;
      end
      INTERRUPT: begin
        if (clear_interrupt) next_state = IDLE;
        else next_state = INTERRUPT;
      end
    endcase
  end

  assign read_valid = cycle_counter > '0 && !m1_waitrequest && m1_readdatavalid;
  assign writing_done = !m1_waitrequest;
  // firmware chose this order
  assign {voxel_y, voxel_z, voxel_x} = voxel_num - 1'b1;
  assign {row, col} = pixel_num - 1'b1;

  always_ff @(posedge clock, posedge reset) begin
    if (reset) begin
      m1_address <= '0;
      cycle_counter <= '0;
      voxel_num <= '0;
      entry_num <= '0;
      pixel_num <= '0;
    end else begin
      case (state)
        RASTERIZING: begin
          m1_address <= voxel_buffer;
          voxel_num  <= '0;
        end
        FETCH_VOXEL: begin
          if (read_valid) begin
            voxel_id  <= m1_readdata;
            voxel_num <= voxel_num + 1'b1;
          end
          cycle_counter <= cycle_counter + 1'b1;
        end
        RASTERIZE: begin
          if (next_state == FETCH_VOXEL) begin
            cycle_counter <= '0;
            m1_address <= m1_address + 1'b1;
          end
        end
        SHADING: begin
          m1_address <= palette_buffer;
          entry_num  <= '0;
        end
        FETCH_ENTRY: begin
          if (read_valid) begin
            palette_entry <= m1_readdata;
            entry_num <= entry_num + 1'b1;
          end
          cycle_counter <= cycle_counter + 1'b1;
        end
        SHADE: begin
          if (next_state == FETCH_ENTRY) begin
            cycle_counter <= '0;
            m1_address <= m1_address + 1'b1;
          end
        end
        WRITEOUT: begin
          m1_address <= pixel_buffer;
          pixel_num  <= '0;
        end
        FETCH_PIXEL: begin
          pixel_num <= pixel_num + 1'b1;
        end
        WRITE: begin
          if (next_state == FETCH_PIXEL) begin
            m1_address <= m1_address + 1'b1;
          end
        end
      endcase
    end
  end

  always_comb begin
    m1_read = 1'b0;
    m1_write = 1'b0;
    m1_writedata = '0;
    irq = 1'b0;
    do_rasterize = 1'b0;
    do_shade = 1'b0;
    case (state)
      FETCH_VOXEL: begin
        m1_read = 1'b1;
      end
      RASTERIZE: begin
        do_rasterize = voxel_num < voxel_count;
      end
      FETCH_ENTRY: begin
        m1_read = 1'b1;
      end
      SHADE: begin
        do_shade = entry_num < entry_count;
      end
      WRITE: begin
        m1_write = 1'b1;
        m1_writedata = pixel;
      end
      INTERRUPT: begin
        irq = 1'b1;
      end
    endcase
  end

endmodule
