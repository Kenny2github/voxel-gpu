#include <stdint.h>
#include <stdio.h>
#include "hardware/hardware.h"

/*** Static variables ***/

/**
 * @brief Information required to configure one interrupt.
 */
struct irq_handler {
    // IRQ ID handled by this handler
    int irq;
    // callback function when interrupt is enabled
    void (*on_enable)(void);
    // callback function when interrupt happens
    void (*on_irq)(void);
};

static int num_irq_handlers = 0;
static struct irq_handler irq_handlers[32];

static union cpsr_t default_state = {
    .fields = {.M = MODE_SVC, .T = 0, .F = 1, .I = 0}
};

/*** Static functions ***/

// Disable interrupts in SVC mode
static void disable_interrupts(void) {
    union cpsr_t status = default_state;
    status.fields.I = 1; // mask IRQ
    __asm__("msr cpsr, %[ps]" : : [ps] "r"(status.value));
}

// Enable interrupts in SVC mode
static void enable_interrupts(void) {
    union cpsr_t status = default_state;
    status.fields.I = 0; // don't mask IRQ
    __asm__("msr cpsr, %[ps]" : : [ps] "r"(status.value));
}

static void init_irq_stack(void) {
    uintptr_t stack = A9_ONCHIP_END & ~(uintptr_t)0x7;
    union cpsr_t status = default_state;
    status.fields.M = MODE_IRQ;
    status.fields.I = 1; // mask IRQ

    __asm__("msr cpsr, %[ps]" : : [ps] "r"(status.value));
    __asm__("mov sp, %[ps]" : : [ps] "r"(stack));
    status.fields.M = MODE_SVC;
    __asm__("msr cpsr, %[ps]" : : [ps] "r"(status.value));
}

static void enable_irq(int n, int cpu_id) {
    int reg_idx, bit_idx, value;

    reg_idx = n >> 5;
    bit_idx = n & 0x1F;
    value = 1 << bit_idx;
    MPCORE_GIC_DIST->icdiser[reg_idx] |= value;

    value = 1 << cpu_id;
    MPCORE_GIC_DIST->icdiptr[n] = (char)value;
}

static void config_GIC(void) {
    for (int i = 0; i < num_irq_handlers; ++i) {
        enable_irq(irq_handlers[i].irq, 0);
    }

    MPCORE_GIC_CPUIF->iccpmr = 0xFFFF;
    MPCORE_GIC_CPUIF->iccicr = 1;
    MPCORE_GIC_DIST->icddcr = 1;
}

/*** Exception handlers ***/

// IRQ exception handler
void __attribute__((__interrupt__)) __cs3_isr_irq(void) {
    int irq = MPCORE_GIC_CPUIF->icciar;
    int i;
    for (i = 0; i < num_irq_handlers; ++i) {
        if (irq == irq_handlers[i].irq) {
            irq_handlers[i].on_irq();
        }
    }
    if (i >= num_irq_handlers) {
        while (1);
    }
    MPCORE_GIC_CPUIF->icceoir = irq;
}

void __attribute__((__interrupt__)) __cs3_reset(void) {
    while (1);
}

void __attribute__((__interrupt__)) __cs3_isr_undef(void) {
    while (1);
}

void __attribute__((__interrupt__)) __cs3_isr_swi(void) {
    while (1);
}

void __attribute__((__interrupt__)) __cs3_isr_pabort(void) {
    while (1);
}

void __attribute__((__interrupt__)) __cs3_isr_dabort(void) {
    while (1);
}

void __attribute__((__interrupt__)) __cs3_isr_fiq(void) {
    while (1);
}

/*** Exported functions ***/

// Initialize all interrupt infrastructure
void config_interrupts(void) {
    disable_interrupts();
    init_irq_stack();
    config_GIC();
    // call interrupt-enable handlers
    for (int i = 0; i < num_irq_handlers; ++i) {
        if (irq_handlers[i].on_enable != NULL) irq_handlers[i].on_enable();
    }
    enable_interrupts();
}

// Configure handler for a type of interrupt
void config_interrupt(int irq, void (*on_enable)(void), void (*on_irq)(void)) {
    irq_handlers[num_irq_handlers].irq = irq;
    irq_handlers[num_irq_handlers].on_enable = on_enable;
    irq_handlers[num_irq_handlers].on_irq = on_irq;
    ++num_irq_handlers;
}
