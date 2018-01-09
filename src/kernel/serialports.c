#include "sos.h"

COMPort port1 = {.port = 0x3f8};
COMPort port2 = {.port = 0x2f8};
COMPort port3 = {.port = 0x3e8};
COMPort port4 = {.port = 0x2e8};


void serials_init(){
        init_serial(port1);
        init_serial(port2);
        init_serial(port3);
        init_serial(port4);
}

void init_serial(COMPort port){
        int PORT = port.port;
        outportb(PORT + 1, 0x00);    // Disable all interrupts
        outportb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
        outportb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
        outportb(PORT + 1, 0x00);    //                  (hi byte)
        outportb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
        outportb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
        outportb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received(COMPort port) {
        int PORT = port.port;
   return inportb(PORT + 5) & 1;
}

char read_serial(COMPort port) {
        int PORT = port.port;
   while (serial_received(port) == 0);
   return inportb(PORT);
}

int is_transmit_empty(COMPort port) {
        int PORT = port.port;
   return inportb(PORT + 5) & 0x20;
}

void write_serial(COMPort port,char a) {
        int PORT = port.port;
   while (is_transmit_empty(port) == 0);

   outportb(PORT,a);
}

