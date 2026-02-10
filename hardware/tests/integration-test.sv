import gpu::*;
`timescale 1ns / 100ps

module mock_ocram #(
    parameter MEM_SIZE = 262144
) (
    input logic clk,
    input logic reset,
    input logic write,
    input logic chipselect,
    input logic [$clog2(MEM_SIZE / 4)-1:0] address,
    input logic [3:0] byteenable,
    output logic [31:0] readdata,
    input logic [31:0] writedata
);
  logic [31:0] mem[0:MEM_SIZE-1];

  always_ff @(posedge clk, posedge reset, posedge chipselect) begin
    if (reset) mem <= '{default: 0};
    else begin
      if (chipselect && write) begin
        unique case (byteenable)
          4'b0000: begin
          end
          4'b0001: begin
            mem[address][7:0] <= writedata[7:0];
          end
          4'b0010: begin
            mem[address][15:8] <= writedata[7:0];
          end
          4'b0011: begin
            mem[address][15:0] <= writedata[15:0];
          end
          4'b0100: begin
            mem[address][23:16] <= writedata[7:0];
          end
          4'b0101: begin
            {mem[address][23:16], mem[address][7:0]} <= writedata[15:0];
          end
          4'b0110: begin
            mem[address][23:8] <= writedata[15:0];
          end
          4'b0111: begin
            mem[address][23:0] <= writedata[23:0];
          end
          4'b1000: begin
            mem[address][31:24] <= writedata[7:0];
          end
          4'b1001: begin
            {mem[address][31:24], mem[address][7:0]} <= writedata[15:0];
          end
          4'b1010: begin
            {mem[address][31:24], mem[address][15:8]} <= writedata[15:0];
          end
          4'b1011: begin
            {mem[address][31:24], mem[address][15:0]} <= writedata[23:0];
          end
          4'b1100: begin
            mem[address][31:16] <= writedata[15:0];
          end
          4'b1101: begin
            {mem[address][31:16], mem[address][7:0]} <= writedata[23:0];
          end
          4'b1110: begin
            mem[address][31:8] <= writedata[23:0];
          end
          4'b1111: begin
            mem[address] <= writedata;
          end
        endcase
      end
    end
  end

  always_comb begin
    readdata = 'x;
    if (chipselect && !write) begin
      unique case (byteenable)
        4'b0000: begin
        end
        4'b0001: begin
          readdata[7:0] = mem[address][7:0];
        end
        4'b0010: begin
          readdata[7:0] = mem[address][15:8];
        end
        4'b0011: begin
          readdata[15:0] = mem[address][15:0];
        end
        4'b0100: begin
          readdata[7:0] = mem[address][23:16];
        end
        4'b0101: begin
          readdata[15:0] = {mem[address][23:16], mem[address][7:0]};
        end
        4'b0110: begin
          readdata[15:0] = mem[address][23:8];
        end
        4'b0111: begin
          readdata[23:0] = mem[address][23:0];
        end
        4'b1000: begin
          readdata[7:0] = mem[address][31:24];
        end
        4'b1001: begin
          readdata[15:0] = {mem[address][31:24], mem[address][7:0]};
        end
        4'b1010: begin
          readdata[15:0] = {mem[address][31:24], mem[address][15:8]};
        end
        4'b1011: begin
          readdata[23:0] = {mem[address][31:24], mem[address][15:0]};
        end
        4'b1100: begin
          readdata[15:0] = mem[address][31:16];
        end
        4'b1101: begin
          readdata[23:0] = {mem[address][31:16], mem[address][7:0]};
        end
        4'b1110: begin
          readdata[23:0] = mem[address][31:8];
        end
        4'b1111: begin
          readdata = mem[address];
        end
      endcase
    end
  end
endmodule

module testbench #(
    parameter OCRAM_BASE = 'h08000000,
    parameter OCRAM_SIZE = 262144
) ();
  logic [ 7:0] s1_address;
  logic        s1_read;
  logic [31:0] s1_readdata;
  logic [31:0] s1_writedata;
  logic        s1_write;
  logic        s1_waitrequest;
  logic        reset;
  logic        clock;
  logic        irq;
  logic [31:0] m1_address;
  logic [15:0] m1_writedata;
  logic        m1_write;
  logic        m1_waitrequest;

  logic [31:0] ocram_readdata;
  logic is_ocram_address;
  assign is_ocram_address = (m1_address >= OCRAM_BASE && m1_address < (OCRAM_BASE + OCRAM_SIZE));
  assign m1_waitrequest   = (is_ocram_address ? 1'b0 : 1'b1);

  voxel_gpu #() DUT (.*);
  mock_ocram #(
      .MEM_SIZE(OCRAM_SIZE)
  ) ocram (
      .clk(clock),
      .reset,
      .write(m1_write),
      .chipselect(is_ocram_address && m1_write),
      .address(m1_address[17:2] - OCRAM_BASE[17:2]),
      .byteenable({m1_address[0], m1_address[0], !m1_address[0], !m1_address[0]}),
      .readdata(ocram_readdata),
      .writedata({16'b0, m1_writedata})
  );

  // set up clock
  initial begin
    clock <= 1'b0;
    forever begin
      #5 clock <= ~clock;
    end
  end

  task read_s1(input logic [7:0] addr, output logic [31:0] data);
    begin
      s1_address = addr;
      s1_read = 1'b1;
      @(posedge clock);
      s1_read = 1'b0;
      data = s1_readdata;
    end
  endtask

  task write_s1(input logic [7:0] addr, input logic [31:0] data);
    begin
      s1_address = addr;
      s1_write = 1'b1;
      s1_writedata = data;
      @(posedge clock);
      s1_write = 1'b0;
    end
  endtask

  task clear_irq;
    logic [31:0] _readdata;
    begin
      @(posedge irq);
      read_s1(15, _readdata);
      if (_readdata) $stop;
    end
  endtask

  int row, col;
  initial begin
    // default values for inputs
    s1_address = '0;
    s1_read = 1'b0;
    s1_writedata = '0;
    s1_write = 1'b0;
    reset = 1'b1;
    // m1 inputs set by blocks above
    @(negedge clock);
    @(negedge clock);
    reset = 1'b0;

    // set up camera at (4, 1, 1)
    // render plane at (2, 3, 0), (2, 3, 6), (2, 0, 0), (2, 0, 6)
    write_s1(16, {8'd4, 8'b0});  // cam.pos.x
    write_s1(17, {8'd1, 8'b0});  // cam.pos.y
    write_s1(18, {8'd1, 8'b0});  // cam.pos.z
    write_s1(19, {8'(-2), 8'd0});  // cam.look0.x
    write_s1(20, {8'd2, 8'd0});  // cam.look0.y
    write_s1(21, {8'(-1), 8'd0});  // cam.look0.z
    write_s1(22, {8'(-2), 8'd0});  // cam.look1.x
    write_s1(23, {8'd2, 8'd0});  // cam.look1.y
    write_s1(24, {8'd5, 8'd0});  // cam.look1.z
    write_s1(25, {8'(-2), 8'd0});  // cam.look2.x
    write_s1(26, {8'(-1), 8'd0});  // cam.look2.y
    write_s1(27, {8'(-1), 8'd0});  // cam.look2.z
    write_s1(28, {8'(-2), 8'd0});  // cam.look3.x
    write_s1(29, {8'(-1), 8'd0});  // cam.look3.y
    write_s1(30, {8'd5, 8'd0});  // cam.look3.z

    for (int i = 0; i < DUT.H_RESOLUTION * DUT.V_RESOLUTION; i += DUT.NUM_SHADERS) begin
      // select chunk
      write_s1(3, i);
      clear_irq();

      write_s1(0, {10'd0, 10'd0, 10'd0, 2'd1});
      clear_irq();

      write_s1(1, {16'd11, 14'd0, 2'd1});
      clear_irq();

      for (int j = i; j < DUT.NUM_SHADERS; ++j) begin
        row = j / DUT.H_RESOLUTION;
        col = j % DUT.H_RESOLUTION;
        write_s1(2, OCRAM_BASE + {8'(row), 9'(col), 1'b0});
        clear_irq();
      end
    end

    $writememh("ocram.hex", ocram.mem);
    $system("./ocram_to_bmp.py");
    $stop;
  end
endmodule
