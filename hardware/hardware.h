#ifndef HARDWARE_H
#define HARDWARE_H

#include <stddef.h>
#include <stdint.h>

// NOTE: In little-endian ARM, struct bitfields are in LSB-to-MSB order;
// volatile structs make all their members volatile; and
// volatile ints must always be accessed by their whole width.
// Remember that unused bits and paddings are wasted by the MMIO allocations,
// not by global variable allocation.

struct __attribute__((__packed__, __aligned__(4))) _vec3 {
    uint32_t x, y, z;
};

struct __attribute__((__packed__, __aligned__(4))) gpu_registers {
    /**
	 * Address of first pixel buffer
	 */
    unsigned char *pixel_buffer1;
    /**
	 * Address of second pixel buffer
	 */
    unsigned char *pixel_buffer2;
    /**
     * Starting address of voxel data buffer
     */
    unsigned char *voxel_buffer;
    /**
     * Position and orientation of the camera
     */
    struct {
        struct _vec3 pos;
        // "look-at" direction for top left pixel
        struct _vec3 top_left;
        // "look-at" direction for top left pixel
        struct _vec3 bottom_right;
    } camera;
    /**
     * WRITE ONLY - write 1 to this register to begin rendering
     */
    uint32_t do_render;
};
extern volatile struct gpu_registers *const GPU;
#define GPU_IRQ 16U

extern volatile unsigned char *const DDR_BASE;
#define DDR_END 0x3FFFFFFF

extern volatile unsigned char *const A9_ONCHIP_BASE;
#define A9_ONCHIP_END 0xFFFFFFFF

extern volatile unsigned char *const SDRAM_BASE;
#define SDRAM_END 0xC3FFFFFF

extern volatile unsigned char *const FPGA_PIXEL_BUF_BASE;
#define FPGA_PIXEL_BUF_END 0xC803FFFF

extern volatile unsigned char *const FPGA_CHAR_BASE;
#define FPGA_CHAR_END 0xC9001FFF

struct __attribute__((__packed__, __aligned__(4))) ledr_registers {
    uint32_t ledr : 10;
    uint32_t _unused : 22;
};
extern volatile struct ledr_registers *const LED;
extern volatile struct ledr_registers *const LEDR;

struct __attribute__((__packed__, __aligned__(4))) hex3_hex0_registers {
    uint32_t hex0 : 8;
    uint32_t hex1 : 8;
    uint32_t hex2 : 8;
    uint32_t hex3 : 8;
};
extern volatile struct hex3_hex0_registers *const HEX3_HEX0;

struct __attribute__((__packed__, __aligned__(4))) hex5_hex4_registers {
    uint32_t hex4 : 8;
    uint32_t hex5 : 8;
    uint32_t _unused : 16;
};
extern volatile struct hex5_hex4_registers *const HEX5_HEX4;

struct __attribute__((__packed__, __aligned__(4))) sw_registers {
    uint32_t sw : 10;
    uint32_t _unused : 22;
};
extern volatile struct sw_registers *const SW;

struct __attribute__((__packed__, __aligned__(4))) key_registers {
    uint32_t key : 4;
    uint32_t : 28;
    uint32_t : 32;
    uint32_t mask : 4;
    uint32_t : 28;
    uint32_t edge : 4;
    uint32_t : 28;
};
extern volatile struct key_registers *const KEY;
#define KEY_IRQ 1U

struct __attribute__((__packed__, __aligned__(4))) ps2_registers {
    uint32_t data : 8;
    uint32_t : 7;
    uint32_t rvalid : 1;
    uint32_t ravail : 16;
    uint32_t re : 1;
    uint32_t : 7;
    uint32_t ri : 1;
    uint32_t : 1;
    uint32_t ce : 1;
    uint32_t : 21;
};
extern volatile struct ps2_registers *const PS2;
#define PS2_IRQ 7U
extern volatile struct ps2_registers *const PS2_DUAL;
#define PS2_DUAL_IRQ 23U

struct __attribute__((__packed__, __aligned__(4))) jtag_uart_registers {
    uint32_t data : 8;
    uint32_t : 7;
    uint32_t rvalid : 1;
    uint32_t ravail : 16;
    uint32_t re : 1;
    uint32_t we : 1;
    uint32_t : 6;
    uint32_t ri : 1;
    uint32_t wi : 1;
    uint32_t ac : 1;
    uint32_t : 5;
    uint32_t wspace : 16;
};
extern volatile struct jtag_uart_registers *const JTAG_UART;
extern volatile struct jtag_uart_registers *const JTAG_UART_2;
#define JTAG_UART_IRQ 8U

struct __attribute__((__packed__, __aligned__(4))) timer_registers {
    uint32_t to : 1;
    uint32_t run : 1;
    uint32_t : 30;
    uint32_t ito : 1;
    uint32_t cont : 1;
    uint32_t start : 1;
    uint32_t stop : 1;
    uint32_t : 28;
    uint32_t start_low : 16;
    uint32_t : 16;
    uint32_t start_high : 16;
    uint32_t : 16;
    uint32_t snapshot_low : 16;
    uint32_t : 16;
    uint32_t snapshot_high : 16;
    uint32_t : 16;
};
extern volatile struct timer_registers *const TIMER;
extern volatile struct timer_registers *const TIMER_2;
#define TIMER_IRQ 0U
#define TIMER_2_IRQ 2U

struct __attribute__((__packed__, __aligned__(4))) buf_ctrl_registers {
    unsigned char *buffer;
    unsigned char *back_buffer;
    uint32_t x_resolution : 16;
    uint32_t y_resolution : 16;
    struct __attribute__((__packed__, __aligned__(4))) {
        uint32_t s : 1;
        uint32_t a : 1;
        uint32_t en : 1;
        uint32_t : 3;
        uint32_t sb : 2;
        uint32_t bs : 4;
        uint32_t : 4;
        uint32_t n : 8;
        uint32_t m : 8;
    } status;
};
extern volatile struct buf_ctrl_registers *const PIXEL_BUF_CTRL;
extern volatile struct buf_ctrl_registers *const CHAR_BUF_CTRL;

struct __attribute__((__packed__, __aligned__(4))) hps_timer_registers {
    uint32_t load;
    uint32_t counter;
    uint32_t e : 1;
    uint32_t m : 1;
    uint32_t i : 1;
    uint32_t : 29;
    uint32_t f : 1;
    uint32_t : 31;
    uint32_t s : 1;
    uint32_t : 31;
};
extern volatile struct hps_timer_registers *const HPS_TIMER[4];
#define HPS_TIMER_0_IRQ 199
#define HPS_TIMER_1_IRQ 200
#define HPS_TIMER_2_IRQ 201
#define HPS_TIMER_3_IRQ 202

struct __attribute__((__packed__, __aligned__(4))) fpga_bridge_registers {
    uint32_t hps2fpga_reset : 1;
    uint32_t lwhps2fpga_reset : 1;
    uint32_t fpga2hps_reset : 1;
	uint32_t : 29;
};
extern volatile struct fpga_bridge_registers *const FPGA_BRIDGE;

struct __attribute__((__packed__, __aligned__(4))) private_timer_registers {
    uint32_t load;
    uint32_t counter;
    uint32_t e : 1;
    uint32_t a : 1;
    uint32_t i : 1;
	uint32_t : 5;
	uint32_t prescaler : 8;
    uint32_t : 16;
    uint32_t f : 1;
    uint32_t : 31;
};
extern volatile struct private_timer_registers *const MPCORE_PRIV_TIMER;
#define PRIVATE_TIMER_IRQ 29

struct __attribute__((__packed__, __aligned__(4))) gic_cpuif_registers {
	/* CPU interface control register */
	uint32_t iccicr;
	/* interrupt priority mask register */
	uint32_t iccpmr;
	/* interrupt acknowledge register */
	uint32_t icciar;
	/* end of interrupt register */
	uint32_t icceoir;
};
extern volatile struct gic_cpuif_registers *const MPCORE_GIC_CPUIF;

struct __attribute__((__packed__, __aligned__(4))) gic_dist_registers {
	/* distributor control register */
	uint32_t icddcr;
	uint32_t icdictr;
	uint32_t icdiidr;
	uint32_t _reserved_0x00C_0x07C[(0x07C - 0x00C) / 4 + 1];
	uint32_t icdisr[32];
	/* interrupt set-enable registers */
	uint32_t icdiser[32];
	/* interrupt clear-enable registers */
	uint32_t icdicer[32];
	uint32_t icdispr[32];
	uint32_t icdicpr[32];
	uint32_t icdabr[32];
	uint32_t _reserved_0x380_0x3FC[(0x3FC - 0x380) / 4 + 1];
	uint8_t icdipr[1020];
	uint32_t _reserved_0x7FC_0x7FC[(0x7FC - 0x7FC) / 4 + 1];
	/* interrupt processor targets registers */
	uint8_t icdiptr[1020];
	uint32_t _reserved_0xBFC_0xBFC[(0xBFC - 0xBFC) / 4 + 1];
	/* interrupt configuration registers */
	uint32_t icdicfr[64];
};
extern volatile struct gic_dist_registers *const MPCORE_GIC_DIST;
_Static_assert(
    offsetof(struct gic_dist_registers, icddcr) == 0, "Wrong ICDDCR offset"
);
_Static_assert(
    offsetof(struct gic_dist_registers, icdiser) == 0x100, "Wrong ICDISER offset"
);
_Static_assert(
    offsetof(struct gic_dist_registers, icdicer) == 0x180, "Wrong ICDICER offset"
);
_Static_assert(
    offsetof(struct gic_dist_registers, icdiptr) == 0x800, "Wrong ICDIPTR offset"
);
_Static_assert(
    offsetof(struct gic_dist_registers, icdicfr) == 0xC00, "Wrong ICDICFR offset"
);

union cpsr_t {
    uint32_t value;
    struct {
        // processor mode
        uint32_t M : 5;
        // shall I enable Thumb execution?
        uint32_t T : 1;
        // shall I mask (disable) FIQ (fast-execution interrupts)?
        uint32_t F : 1;
        // shall I mask (disable) IRQ (interrupts)?
        uint32_t I : 1;
        // shall I mask (disable) asynchronous aborts?
        uint32_t A : 1;
        // shall I use big-endian operation?
        uint32_t E : 1;
        // used in Thumb execution
        uint32_t IT_7_2 : 6;
        // Greater than or Equal flags
        uint32_t GE : 4;
        uint32_t _reserved : 4;
        // shall I enable Jazelle execution?
        uint32_t J : 1;
        // used in Thumb execution
        uint32_t IT_1_0 : 2;
        // saturation flag
        uint32_t Q : 1;
        // overflow flag
        uint32_t V : 1;
        // carry flag
        uint32_t C : 1;
        // zero flag
        uint32_t Z : 1;
        // negative flag
        uint32_t N : 1;
    } fields;
};
#define MODE_SVC 0b10011
#define MODE_IRQ 0b10010

#endif
