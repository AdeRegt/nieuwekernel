#include "sos.h"
//
// I N T   H A N D E L I N G
// http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
//

#define IDT_SIZE 256

extern void irq_defaulte();
extern void irq_error();

/* Defines an IDT entry */


struct idt_entry {
    unsigned short base_lo;
    unsigned short sel; /* Our kernel segment goes here! */
    unsigned char always0; /* This will ALWAYS be set to 0! */
    unsigned char flags; /* Set using the above table! */
    unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[IDT_SIZE];
struct idt_ptr idtp;

void setInterrupt(int i, unsigned long base) {
    idt[i].base_lo = (base & 0xFFFF);
    idt[i].base_hi = (base >> 16) & 0xFFFF;
    idt[i].sel = 0x10;
    idt[i].always0 = 0;
    idt[i].flags = 0x8E;
}


extern void irq_ata();
extern void irq_kbd();
extern void irq_mou();

/**
 * Let the CPU know where the idt is
 */
void lidt() {
    // setup PIC http://www.osdever.net/bkerndev/Docs/whatsleft.htm
    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
    // set all ints
    int i = 0;
    for (i = 0; i < IDT_SIZE; i++) {
        setInterrupt(i, (unsigned long) &irq_defaulte);
    }
    for(i = 0 ; i < 10 ; i++){
        setInterrupt(i, (unsigned long) &irq_error);
    }
    //for(i = 30 ; i < 33 ; i++){//35
    //setInterrupt(32, (unsigned long) &irq_timer);
    //}
    setInterrupt(32+1, (unsigned long) &irq_kbd);
    setInterrupt(32+14, (unsigned long) &irq_ata);
    setInterrupt(32+12, (unsigned long) &irq_mou);
    idtp.limit = (sizeof (struct idt_entry) * IDT_SIZE) - 1;
    idtp.base = (unsigned int) &idt;
    asm volatile("lidt idtp\nsti");
}

