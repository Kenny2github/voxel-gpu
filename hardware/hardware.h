#ifndef HARDWARE_H
#define HARDWARE_H

#include <stddef.h>
#include <stdint.h>

#define assert_word_size(type, name)                                           \
    _Static_assert(sizeof(type) == sizeof(uint32_t), name " must be word size")
#define PA_STRUCT struct __attribute__((__packed__, __aligned__(4)))

// NOTE: In little-endian ARM, struct bitfields are in LSB-to-MSB order;
// volatile structs make all their members volatile; and
// volatile ints must always be accessed by their whole width.
// Remember that unused bits and paddings are wasted by the MMIO allocations,
// not by global variable allocation.

void memcpy_32(volatile uint32_t *dest, uint32_t *src, size_t n);

struct __attribute__((__packed__, __aligned__(4))) _vec3 {
    uint32_t x, y, z;
};

struct __attribute__((__packed__, __aligned__(4))) gpu_registers {
    /**
	 * Starting address of back pixel buffer
	 */
    unsigned char *pixel_buffer;
    /**
     * Starting address of voxel data buffer
     */
    unsigned char *voxel_buffer;
    /**
     * Minimum number of voxels containing all non-empty voxels
     */
    uint32_t voxel_count;
    /**
     * Starting address of palette data buffer
     */
    unsigned char *palette_buffer;
    /**
     * Number of palette entries
     */
    uint32_t palette_length;
    // reserved space
    uint32_t _reserved_0x14_0x3C[10];
    /**
     * WRITE ONLY
     */
    union {
        // write 1 to this register to begin rendering
        uint32_t do_render;
        // write 0 to this register to clear interrupt
        uint32_t render_irq;
    };
    /**
     * Position and orientation of the camera
     */
    struct __attribute__((__packed__, __aligned__(4))) {
        struct _vec3 pos;
        // "look-at" directions for top left, top right,
        // bottom left, and bottom right pixels, in that order
        struct _vec3 look[4];
    } camera;
};
_Static_assert(
    offsetof(struct gpu_registers, do_render) == 0x0f * 4,
    "Wrong register offset"
);
_Static_assert(
    offsetof(struct gpu_registers, camera) == 0x10 * 4, "Wrong camera offset"
);
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

PA_STRUCT ledr_registers {
    uint32_t ledr : 10;
    uint32_t : 22;
};
extern volatile struct ledr_registers *const LED;
extern volatile struct ledr_registers *const LEDR;

PA_STRUCT hex3_hex0_registers {
    uint32_t hex0 : 8;
    uint32_t hex1 : 8;
    uint32_t hex2 : 8;
    uint32_t hex3 : 8;
};
assert_word_size(struct hex3_hex0_registers, "HEX3-0 register");
extern volatile struct hex3_hex0_registers *const HEX3_HEX0;

PA_STRUCT hex5_hex4_registers {
    uint32_t hex4 : 8;
    uint32_t hex5 : 8;
    uint32_t : 16;
};
assert_word_size(struct hex5_hex4_registers, "HEX5-4 register");
extern volatile struct hex5_hex4_registers *const HEX5_HEX4;

PA_STRUCT sw_registers {
    uint32_t sw : 10;
    uint32_t : 22;
};
assert_word_size(struct sw_registers, "SW register");
extern volatile struct sw_registers *const SW;

PA_STRUCT key_bits_register {
    uint32_t bits : 4;
    uint32_t : 28;
};
assert_word_size(struct key_bits_register, "KEY registers");
PA_STRUCT key_registers {
    struct key_bits_register key;
    uint32_t : 32;
    struct key_bits_register mask;
    struct key_bits_register edge;
};
extern volatile struct key_registers *const KEY;
#define KEY_IRQ 1U

PA_STRUCT ps2_data {
    uint32_t data : 8;
    uint32_t : 7;
    uint32_t rvalid : 1;
    uint32_t ravail : 16;
};
assert_word_size(struct ps2_data, "PS2 data type");
PA_STRUCT ps2_flags {
    uint32_t re : 1;
    uint32_t : 7;
    uint32_t ri : 1;
    uint32_t : 1;
    uint32_t ce : 1;
    uint32_t : 21;
};
assert_word_size(struct ps2_flags, "PS2 flags type");
PA_STRUCT ps2_registers {
    struct ps2_data data;
    struct ps2_flags flags;
};
extern volatile struct ps2_registers *const PS2;
#define PS2_IRQ 79U
extern volatile struct ps2_registers *const PS2_DUAL;
#define PS2_DUAL_IRQ 89U

PA_STRUCT jtag_uart_data {
    uint32_t data : 8;
    uint32_t : 7;
    uint32_t rvalid : 1;
    uint32_t ravail : 16;
};
assert_word_size(struct jtag_uart_data, "JTAG UART data type");
PA_STRUCT jtag_uart_flags {
    uint32_t re : 1;
    uint32_t we : 1;
    uint32_t : 6;
    uint32_t ri : 1;
    uint32_t wi : 1;
    uint32_t ac : 1;
    uint32_t : 5;
    uint32_t wspace : 16;
};
assert_word_size(struct jtag_uart_flags, "JTAG UART flags type");
PA_STRUCT jtag_uart_registers {
    struct jtag_uart_data data;
    struct jtag_uart_flags flags;
};
extern volatile struct jtag_uart_registers *const JTAG_UART;
extern volatile struct jtag_uart_registers *const JTAG_UART_2;
#define JTAG_UART_IRQ 8U

PA_STRUCT timer_status_register {
    uint32_t to : 1;
    uint32_t run : 1;
    uint32_t : 30;
};
assert_word_size(
    struct timer_status_register, "Interval timer status register type"
);
PA_STRUCT timer_control_register {
    uint32_t ito : 1;
    uint32_t cont : 1;
    uint32_t start : 1;
    uint32_t stop : 1;
    uint32_t : 28;
};
assert_word_size(
    struct timer_control_register, "Interval timer control register type"
);
PA_STRUCT timer_registers {
    struct timer_status_register status;
    struct timer_control_register control;
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

PA_STRUCT buf_ctrl_status_register {
    uint32_t s : 1;
    uint32_t a : 1;
    uint32_t en : 1;
    uint32_t : 3;
    uint32_t sb : 2;
    uint32_t bs : 4;
    uint32_t : 4;
    uint32_t n : 8;
    uint32_t m : 8;
};
assert_word_size(
    struct buf_ctrl_status_register, "Buffer control status register type"
);
PA_STRUCT buf_ctrl_registers {
    unsigned char *buffer;
    unsigned char *back_buffer;
    uint32_t x_resolution : 16;
    uint32_t y_resolution : 16;
    struct buf_ctrl_status_register status;
};
extern volatile struct buf_ctrl_registers *const PIXEL_BUF_CTRL;
extern volatile struct buf_ctrl_registers *const CHAR_BUF_CTRL;

PA_STRUCT hps_timer_control_register {
    uint32_t e : 1;
    uint32_t m : 1;
    uint32_t i : 1;
    uint32_t : 29;
};
assert_word_size(
    struct hps_timer_control_register, "HPS timer control register type"
);
PA_STRUCT hps_timer_registers {
    uint32_t load;
    uint32_t counter;
    struct hps_timer_control_register control;
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

PA_STRUCT fpga_bridge_registers {
    uint32_t hps2fpga_reset : 1;
    uint32_t lwhps2fpga_reset : 1;
    uint32_t fpga2hps_reset : 1;
    uint32_t : 29;
};
assert_word_size(struct fpga_bridge_registers, "FPGA bridge register type");
extern volatile struct fpga_bridge_registers *const FPGA_BRIDGE;

PA_STRUCT private_timer_control_register {
    uint32_t e : 1;
    uint32_t a : 1;
    uint32_t i : 1;
    uint32_t : 5;
    uint32_t prescaler : 8;
    uint32_t : 16;
};
assert_word_size(
    struct private_timer_control_register, "Private timer control register type"
);
PA_STRUCT private_timer_registers {
    uint32_t load;
    uint32_t counter;
    struct private_timer_control_register control;
    uint32_t f : 1;
    uint32_t : 31;
};
extern volatile struct private_timer_registers *const MPCORE_PRIV_TIMER;
#define PRIVATE_TIMER_IRQ 29

struct __attribute__((__packed__)) gic_cpuif_registers {
    /* CPU interface control register */
    uint32_t iccicr;
    /* interrupt priority mask register */
    uint32_t iccpmr;
    /* 4-byte offset */
    uint32_t _reserved0;
    /* interrupt acknowledge register */
    uint32_t icciar;
    /* end of interrupt register */
    uint32_t icceoir;
};
extern volatile struct gic_cpuif_registers *const MPCORE_GIC_CPUIF;

PA_STRUCT gic_dist_registers {
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
    offsetof(struct gic_dist_registers, icdiser) == 0x100,
    "Wrong ICDISER offset"
);
_Static_assert(
    offsetof(struct gic_dist_registers, icdicer) == 0x180,
    "Wrong ICDICER offset"
);
_Static_assert(
    offsetof(struct gic_dist_registers, icdiptr) == 0x800,
    "Wrong ICDIPTR offset"
);
_Static_assert(
    offsetof(struct gic_dist_registers, icdicfr) == 0xC00,
    "Wrong ICDICFR offset"
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
assert_word_size(union cpsr_t, "CPSR register type");
#define MODE_SVC 0b10011
#define MODE_IRQ 0b10010

#endif
