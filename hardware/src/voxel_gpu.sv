`include "common.svh"

module voxel_gpu #(
    parameter H_RESOLUTION = 256,
    parameter V_RESOLUTION = 192,
    parameter PIXEL_BITS   = 16
) (
    input  wire [ 7:0] s1_address,       //    s1.address
    input  wire        s1_read,          //      .read
    output wire [31:0] s1_readdata,      //      .readdata
    input  wire [31:0] s1_writedata,     //      .writedata
    input  wire        s1_write,         //      .write
    output wire        s1_waitrequest,   //      .waitrequest
    input  wire        reset,            // reset.reset
    input  wire        clock,            // clock.clk
    output wire        irq,              //   irq.irq
    output wire [31:0] m1_address,       //    m1.address
    output wire [ 7:0] m1_writedata,     //      .writedata
    output wire        m1_write,         //      .write
    input  wire        m1_waitrequest,   //      .waitrequest
    input  wire [ 7:0] m1_readdata,      //      .readdata
    output wire        m1_read,          //      .read
    input  wire        m1_readdatavalid  //      .readdatavalid
);
  // GPU.*
  logic [31:0] pixel_buffer, voxel_buffer, voxel_count, palette_buffer, palette_length;
  // write-only signals cleared automatically
  logic do_render, clear_interrupt;
  // GPU.camera
  camera cam;

  gpu_controller #(
      .STOP_ROW  (V_RESOLUTION),
      .STOP_COL  (H_RESOLUTION),
      .TOTAL_ROWS(V_RESOLUTION),
      .TOTAL_COLS(H_RESOLUTION),
      .PIXEL_BITS(PIXEL_BITS)
  ) ctrl (
      .*
  );

  always_ff @(posedge clock or posedge reset) begin
    if (reset) begin
      pixel_buffer <= DEFAULT_BUFFER;
      voxel_buffer <= '0;
      voxel_count <= '0;
      palette_buffer <= '0;
      palette_length <= '0;
      do_render <= 1'b0;
      clear_interrupt <= 1'b0;
      cam <= '{default: 0};
    end else if (s1_write) begin
      case (s1_address)
        8'h00: begin
          pixel_buffer <= s1_writedata;
        end
        8'h01: begin
          voxel_buffer <= s1_writedata;
        end
        8'h02: begin
          voxel_count <= s1_writedata;
        end
        8'h03: begin
          palette_buffer <= s1_writedata;
        end
        8'h04: begin
          palette_length <= s1_writedata;
        end
        8'h0f: begin
          // write 1: do_render; write 0: clear_interrupt
          if (s1_writedata) do_render <= 1'b1;
          else clear_interrupt <= 1'b1;
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
    end else begin
      do_render <= 1'b0;
      clear_interrupt <= 1'b0;
    end
  end

  always_comb begin
    s1_readdata = '0;
    case (s1_address)
      8'h00: begin
        s1_readdata = pixel_buffer;
      end
      8'h01: begin
        s1_readdata = voxel_buffer;
      end
      8'h02: begin
        s1_readdata = voxel_count;
      end
      8'h03: begin
        s1_readdata = palette_buffer;
      end
      8'h04: begin
        s1_readdata = palette_length;
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
  assign m1_write = 1'b0;
  assign m1_writedata = 32'b0;
  assign m1_address = 32'b0;
  assign m1_read = 1'b0;
  assign irq = 1'b0;

endmodule
