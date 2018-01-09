/* Input a byte from a port */
unsigned char inportb(unsigned int port){
   unsigned char ret;
   asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
   return ret;
}

/* Input a word from a port */
unsigned short inportw(unsigned int port){
        unsigned short ret;
        asm volatile ("inw %%dx,%%ax":"=a"(ret):"d"(port));
//      asm volatile ("inw %1, %0" : "=a" (rv) : "dN"(port));
        return ret;
}

//unsigned long inportd(unsigned int port){
//        unsigned long ret;
//        asm volatile ("ind %%dx,%%eax":"=a"(ret):"d"(port));
//        return ret;
//}
/* Output a byte to a port */
/* July 6, 2001 added space between :: to make code compatible with gpp */

void outportb(unsigned int port,unsigned char value){
   asm volatile ("outb %%al,%%dx": :"d" (port), "a" (value));
}

void outportw(unsigned int port,unsigned short value){
        asm volatile("outw %%ax,%%dx": :"d"(port),"a"(value));
}

