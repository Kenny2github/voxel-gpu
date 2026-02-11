import gpu::*;

module voxel_gpu #(
    parameter H_RESOLUTION = 320,
    parameter V_RESOLUTION = 240,
    parameter NUM_SHADERS  = H_RESOLUTION,
    parameter COORD_BITS   = 10,
    parameter FRACT_BITS   = COORD_BITS,
    parameter PIXEL_BITS   = 16
) (
    input  logic [ 7:0] s1_address,      //    s1.address
    input  logic        s1_read,         //      .read
    output logic [31:0] s1_readdata,     //      .readdata
    input  logic [31:0] s1_writedata,    //      .writedata
    input  logic        s1_write,        //      .write
    output logic        s1_waitrequest,  //      .waitrequest
    input  logic        reset,           // reset.reset
    input  logic        clock,           // clock.clk
    output logic        irq,             //   irq.irq
    output logic [31:0] m1_address,      //    m1.address
    output logic [15:0] m1_writedata,    //      .writedata
    output logic        m1_write,        //      .write
    input  logic        m1_waitrequest   //      .waitrequest
);
  localparam VOXEL_BITS = 32 - (COORD_BITS * 3);
  enum logic [3:0] {
    IDLE,
    COORDINATE,
    RAYCAST,
    RASTERIZE,
    SHADE,
    WRITE_OUT,
    INTERRUPT,
    ERROR
  } state;

  logic ready;
  assign ready = (state == IDLE) || (state == INTERRUPT);

  // GPU.*
  logic [31:0] rasterize_voxel, shade_entry, write_pixel, start_pixel;
  // GPU.camera
  camera cam;

  // local variables
  logic [31:0] cycle_counter;

  // shader variables
  localparam ROW_BITS = $clog2(V_RESOLUTION);
  localparam COL_BITS = $clog2(H_RESOLUTION);
  logic signed [COORD_BITS-1:0] voxel_x, voxel_y, voxel_z;
  logic [(32-COORD_BITS*3)-1:0] voxel_id;
  assign {voxel_x, voxel_y, voxel_z, voxel_id} = rasterize_voxel;
  logic [PIXEL_BITS-1:0] palette_entry;
  assign palette_entry = shade_entry[31-:PIXEL_BITS];
  logic [ROW_BITS+COL_BITS-1:0] pixel_index;
  assign pixel_index = write_pixel[(COL_BITS + 1) +: ROW_BITS] * H_RESOLUTION + write_pixel[1 +: COL_BITS] - start_pixel;
  wire [PIXEL_BITS-1:0] pixel;
  logic raycast_start, coordinate_start, do_rasterize, do_shade;
  wand raycast_valid, coordinate_valid, rasterizing_done, shading_done;
  wor error;

  genvar i;
  generate
    for (i = 0; i < NUM_SHADERS; ++i) begin
      // compute row and column of shader
      logic [ROW_BITS+COL_BITS:0] div_i_val;
      logic [ROW_BITS-1:0] shader_row;
      logic [COL_BITS-1:0] shader_col;
      div #(
          // + 1 sign bit
          .WIDTH(ROW_BITS + COL_BITS + 1),
          .FBITS(0)
      ) div_i (
          .clk(clock),
          .rst(reset),
          .start(coordinate_start),
          .valid(coordinate_valid),
          .busy(),
          .done(coordinate_valid),
          .dbz(error),
          .ovf(error),
          .a((ROW_BITS + COL_BITS + 1)'(i + start_pixel)),
          .b((ROW_BITS + COL_BITS + 1)'(H_RESOLUTION)),
          .val(div_i_val)
      );
      assign shader_row = div_i_val[ROW_BITS-1:0];
      assign shader_col = (i + start_pixel) - (shader_row * H_RESOLUTION);
      // compute camera look direction
      logic signed [COORD_BITS+FRACT_BITS-1:0] cam_look_x, cam_look_y, cam_look_z;
      logic signed [ROW_BITS+COL_BITS+FRACT_BITS:0] lerp2_x_val, lerp2_y_val, lerp2_z_val;
      lerp2 #(
          .WIDTH(ROW_BITS + COL_BITS + 1 + FRACT_BITS),
          .FBITS(FRACT_BITS)
      ) lerp2_x (
          .p0({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look0.x[COORD_BITS+FRACT_BITS-1]}},
            cam.look0.x[COORD_BITS+FRACT_BITS-1:0]
          }),
          .p1({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look1.x[COORD_BITS+FRACT_BITS-1]}},
            cam.look1.x[COORD_BITS+FRACT_BITS-1:0]
          }),
          .p2({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look2.x[COORD_BITS+FRACT_BITS-1]}},
            cam.look2.x[COORD_BITS+FRACT_BITS-1:0]
          }),
          .p3({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look3.x[COORD_BITS+FRACT_BITS-1]}},
            cam.look3.x[COORD_BITS+FRACT_BITS-1:0]
          }),
          .x((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(shader_col << FRACT_BITS)),
          .y((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(shader_row << FRACT_BITS)),
          .X((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(H_RESOLUTION << FRACT_BITS)),
          .Y((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(V_RESOLUTION << FRACT_BITS)),
          .val(lerp2_x_val),
          .start(raycast_start),
          .done(raycast_valid),
          .error,
          .clock,
          .reset
      );
      assign cam_look_x = lerp2_x_val[COORD_BITS+FRACT_BITS-1:0];
      lerp2 #(
          .WIDTH(ROW_BITS + COL_BITS + 1 + FRACT_BITS),
          .FBITS(FRACT_BITS)
      ) lerp2_y (
          .p0({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look0.y[COORD_BITS+FRACT_BITS-1]}},
            cam.look0.y[COORD_BITS+FRACT_BITS-1:0]
          }),
          .p1({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look1.y[COORD_BITS+FRACT_BITS-1]}},
            cam.look1.y[COORD_BITS+FRACT_BITS-1:0]
          }),
          .p2({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look2.y[COORD_BITS+FRACT_BITS-1]}},
            cam.look2.y[COORD_BITS+FRACT_BITS-1:0]
          }),
          .p3({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look3.y[COORD_BITS+FRACT_BITS-1]}},
            cam.look3.y[COORD_BITS+FRACT_BITS-1:0]
          }),
          .x((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(shader_col << FRACT_BITS)),
          .y((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(shader_row << FRACT_BITS)),
          .X((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(H_RESOLUTION << FRACT_BITS)),
          .Y((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(V_RESOLUTION << FRACT_BITS)),
          .val(lerp2_y_val),
          .start(raycast_start),
          .done(raycast_valid),
          .error,
          .clock,
          .reset
      );
      assign cam_look_y = lerp2_y_val[COORD_BITS+FRACT_BITS-1:0];
      lerp2 #(
          .WIDTH(ROW_BITS + COL_BITS + 1 + FRACT_BITS),
          .FBITS(FRACT_BITS)
      ) lerp2_z (
          .p0({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look0.z[COORD_BITS+FRACT_BITS-1]}},
            cam.look0.z[COORD_BITS+FRACT_BITS-1:0]
          }),
          .p1({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look1.z[COORD_BITS+FRACT_BITS-1]}},
            cam.look1.z[COORD_BITS+FRACT_BITS-1:0]
          }),
          .p2({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look2.z[COORD_BITS+FRACT_BITS-1]}},
            cam.look2.z[COORD_BITS+FRACT_BITS-1:0]
          }),
          .p3({
            {(ROW_BITS + COL_BITS - COORD_BITS + 1) {cam.look3.z[COORD_BITS+FRACT_BITS-1]}},
            cam.look3.z[COORD_BITS+FRACT_BITS-1:0]
          }),
          .x((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(shader_col << FRACT_BITS)),
          .y((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(shader_row << FRACT_BITS)),
          .X((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(H_RESOLUTION << FRACT_BITS)),
          .Y((ROW_BITS + COL_BITS + 1 + FRACT_BITS)'(V_RESOLUTION << FRACT_BITS)),
          .val(lerp2_z_val),
          .start(raycast_start),
          .done(raycast_valid),
          .error,
          .clock,
          .reset
      );
      assign cam_look_z = lerp2_z_val[COORD_BITS+FRACT_BITS-1:0];
      pixel_shader #(
          .INDEX(i),
          .INDEX_BITS(ROW_BITS + COL_BITS),
          .COORD_BITS(COORD_BITS),
          .FRACT_BITS(FRACT_BITS),
          .PIXEL_BITS(PIXEL_BITS)
      ) shader (
          .cam_pos_x(cam.pos.x[COORD_BITS+FRACT_BITS-1:0]),
          .cam_pos_y(cam.pos.y[COORD_BITS+FRACT_BITS-1:0]),
          .cam_pos_z(cam.pos.z[COORD_BITS+FRACT_BITS-1:0]),
          .*
      );
    end
  endgenerate

  always_ff @(posedge clock or posedge reset) begin
    if (reset) begin
      state <= IDLE;
      rasterize_voxel <= '0;
      shade_entry <= '0;
      write_pixel <= '0;
      start_pixel <= '0;
      cam <= '{default: 0};
      cycle_counter <= 0;
    end else begin
      if (s1_write) begin
        case (s1_address)
          8'h00: begin
            if (ready) begin
              rasterize_voxel <= s1_writedata;
              state <= RASTERIZE;
            end else begin
              state <= ERROR;
            end
          end
          8'h01: begin
            if (ready) begin
              shade_entry <= s1_writedata;
              state <= SHADE;
            end else begin
              state <= ERROR;
            end
          end
          8'h02: begin
            if (ready) begin
              write_pixel <= s1_writedata;
              state <= WRITE_OUT;
            end else begin
              state <= ERROR;
            end
          end
          8'h03: begin
            if (ready) begin
              start_pixel <= s1_writedata;
              state <= COORDINATE;
            end else begin
              state <= ERROR;
            end
          end
          8'h0f: begin
            if (state == ERROR && s1_writedata) begin
              state <= IDLE;
            end else begin
              state <= ERROR;
            end
          end
          8'h10: begin
            cam.pos.x <= s1_writedata;
          end
          8'h11: begin
            cam.pos.y <= s1_writedata;
          end
          8'h12: begin
            cam.pos.z <= s1_writedata;
          end
          8'h13: begin
            cam.look0.x <= s1_writedata;
          end
          8'h14: begin
            cam.look0.y <= s1_writedata;
          end
          8'h15: begin
            cam.look0.z <= s1_writedata;
          end
          8'h16: begin
            cam.look1.x <= s1_writedata;
          end
          8'h17: begin
            cam.look1.y <= s1_writedata;
          end
          8'h18: begin
            cam.look1.z <= s1_writedata;
          end
          8'h19: begin
            cam.look2.x <= s1_writedata;
          end
          8'h1a: begin
            cam.look2.y <= s1_writedata;
          end
          8'h1b: begin
            cam.look2.z <= s1_writedata;
          end
          8'h1c: begin
            cam.look3.x <= s1_writedata;
          end
          8'h1d: begin
            cam.look3.y <= s1_writedata;
          end
          8'h1e: begin
            cam.look3.z <= s1_writedata;
          end
        endcase
      end
      if (s1_read) begin
        case (s1_address)
          8'h0f: begin
            // reading from status register clears interrupt
            if (state == INTERRUPT) begin
              state <= IDLE;
            end else begin
              state <= ERROR;
            end
          end
        endcase
      end
      cycle_counter <= cycle_counter + 1;
      case (state)
        IDLE: begin
          cycle_counter <= 0;
        end
        COORDINATE: begin
          if (error) begin
            state <= ERROR;
          end else if (coordinate_valid) begin
            state <= RAYCAST;
            cycle_counter <= 0;
          end
        end
        RAYCAST: begin
          if (error) state <= ERROR;
          else if (raycast_valid) state <= INTERRUPT;
        end
        RASTERIZE: begin
          if (rasterizing_done) state <= INTERRUPT;
        end
        SHADE: begin
          if (shading_done) state <= INTERRUPT;
        end
        WRITE_OUT: begin
          if (!m1_waitrequest) state <= INTERRUPT;
        end
        INTERRUPT: begin
          cycle_counter <= 0;
        end
        ERROR: begin
          cycle_counter <= 0;
        end
      endcase
    end
  end

  always_comb begin
    coordinate_start = 1'b0;
    raycast_start = 1'b0;
    do_rasterize = 1'b0;
    do_shade = 1'b0;
    m1_address = '0;
    m1_write = 1'b0;
    m1_writedata = '0;
    case (state)
      COORDINATE: begin
        coordinate_start = cycle_counter < 2;
      end
      RAYCAST: begin
        raycast_start = cycle_counter < 2;
      end
      RASTERIZE: begin
        do_rasterize = cycle_counter < 2;
      end
      SHADE: begin
        do_shade = cycle_counter < 2;
      end
      WRITE_OUT: begin
        m1_address = write_pixel;
        m1_write = 1'b1;
        m1_writedata = pixel;
      end
    endcase
  end

  always_comb begin
    s1_readdata = '0;
    case (s1_address)
      8'h0f: begin
        s1_readdata = ready ? 0 : (state == ERROR ? 2 : 1);
      end
      8'h10: begin
        s1_readdata = cam.pos.x;
      end
      8'h11: begin
        s1_readdata = cam.pos.y;
      end
      8'h12: begin
        s1_readdata = cam.pos.z;
      end
      8'h13: begin
        s1_readdata = cam.look0.x;
      end
      8'h14: begin
        s1_readdata = cam.look0.y;
      end
      8'h15: begin
        s1_readdata = cam.look0.z;
      end
      8'h16: begin
        s1_readdata = cam.look1.x;
      end
      8'h17: begin
        s1_readdata = cam.look1.y;
      end
      8'h18: begin
        s1_readdata = cam.look1.z;
      end
      8'h19: begin
        s1_readdata = cam.look2.x;
      end
      8'h1a: begin
        s1_readdata = cam.look2.y;
      end
      8'h1b: begin
        s1_readdata = cam.look2.z;
      end
      8'h1c: begin
        s1_readdata = cam.look3.x;
      end
      8'h1d: begin
        s1_readdata = cam.look3.y;
      end
      8'h1e: begin
        s1_readdata = cam.look3.z;
      end
      default: begin
        s1_readdata = 32'b0;
      end
    endcase
  end

  assign s1_waitrequest = 1'b0;
  assign irq = (state == INTERRUPT);

endmodule
