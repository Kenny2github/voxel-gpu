#ifndef INTERRUPTS_H
#define INTERRUPTS_H

void config_interrupt(int irq, void (*on_enable)(void), void (*on_irq)(void));
void config_interrupts(void);

#endif