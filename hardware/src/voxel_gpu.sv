import gpu::*;

module voxel_gpu #(
    parameter H_RESOLUTION = 256,
    parameter V_RESOLUTION = 192,
    parameter VOXEL_BITS   = 2,
    parameter COORD_BITS   = 10,
    parameter PIXEL_BITS   = 16
) (
    input  logic [ 7:0] s1_address,       //    s1.address
    input  logic        s1_read,          //      .read
    output logic [31:0] s1_readdata,      //      .readdata
    input  logic [31:0] s1_writedata,     //      .writedata
    input  logic        s1_write,         //      .write
    output logic        s1_waitrequest,   //      .waitrequest
    input  logic        reset,            // reset.reset
    input  logic        clock,            // clock.clk
    output logic        irq,              //   irq.irq
    output logic [31:0] m1_address,       //    m1.address
    output logic [ 7:0] m1_writedata,     //      .writedata
    output logic        m1_write,         //      .write
    input  logic        m1_waitrequest,   //      .waitrequest
    input  logic [ 7:0] m1_readdata,      //      .readdata
    output logic        m1_read,          //      .read
    input  logic        m1_readdatavalid  //      .readdatavalid
);
  enum logic [3:0] {
    IDLE,
    POSITION,
    RASTERIZE,
    SHADE,
    WRITE_OUT,
    INTERRUPT,
    ERROR
  } state;

  logic ready;
  assign ready = (state == IDLE) || (state == INTERRUPT);

  // GPU.*
  logic [31:0] rasterize_voxel, shade_entry, write_pixel;
  logic [15:0] start_row, start_col;
  // GPU.camera
  camera cam;

  always_ff @(posedge clock or posedge reset) begin
    if (reset) begin
      state <= IDLE;
      rasterize_voxel <= '0;
      shade_entry <= '0;
      write_pixel <= '0;
      start_row <= '0;
      start_col <= '0;
      cam <= '{default: 0};
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
              {start_col, start_row} <= s1_writedata;
              state <= POSITION;
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
    end
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
