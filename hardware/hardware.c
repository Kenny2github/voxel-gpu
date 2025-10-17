#include "hardware/hardware.h"

volatile struct gpu_registers *const GPU = (void *)0xFF205800;

volatile unsigned char *const DDR_BASE = (void *)0x00000000;
volatile unsigned char *const A9_ONCHIP_BASE = (void *)0xFFFF0000;
volatile unsigned char *const SDRAM_BASE = (void *)0xC0000000;
volatile unsigned char *const FPGA_PIXEL_BUF_BASE = (void *)0xC8000000;
volatile unsigned char *const FPGA_CHAR_BASE = (void *)0xC9000000;
volatile struct ledr_registers *const LED_BASE = (void *)0xFF200000;
volatile struct ledr_registers *const LEDR_BASE = (void *)0xFF200000;
volatile struct hex3_hex0_registers *const HEX3_HEX0 = (void *)0xFF200020;
volatile struct hex5_hex4_registers *const HEX5_HEX4 = (void *)0xFF200030;
volatile struct sw_registers *const SW = (void *)0xFF200040;
volatile struct key_registers *const KEY = (void *)0xFF200050;
volatile struct ps2_registers *const PS2 = (void *)0xFF200100;
volatile struct ps2_registers *const PS2_DUAL = (void *)0xFF200108;
volatile struct jtag_uart_registers *const JTAG_UART = (void *)0xFF201000;
volatile struct jtag_uart_registers *const JTAG_UART_2 = (void *)0xFF201008;
volatile struct timer_registers *const TIMER = (void *)0xFF202000;
volatile struct timer_registers *const TIMER_2 = (void *)0xFF202020;
volatile struct buf_ctrl_registers *const PIXEL_BUF_CTRL = (void *)0xFF203020;
volatile struct buf_ctrl_registers *const CHAR_BUF_CTRL = (void *)0xFF203030;
volatile struct hps_timer_registers *const HPS_TIMER[4] = {
    (void *)0xFFC08000,
    (void *)0xFFC09000,
    (void *)0xFFD00000,
    (void *)0xFFD01000,
};
volatile struct fpga_bridge_registers *const FPGA_BRIDGE = (void *)0xFFD0501C;
volatile struct private_timer_registers *const MPCORE_PRIV_TIMER = (void *)0xFFFEC600;
volatile struct gic_cpuif_registers *const MPCORE_GIC_CPUIF = (void *)0xFFFEC100;
volatile struct gic_dist_registers *const MPCORE_GIC_DIST = (void *)0xFFFED000;
