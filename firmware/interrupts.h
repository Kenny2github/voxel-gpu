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
// Initialize all interrupt infrastructure
void config_interrupts(void);

// Configure handler for a type of interrupt
void config_interrupt(int irq, void (*on_enable)(void), void (*on_irq)(void));
