#include "sos.h"
//
// H D D   M A N A G E R
//
//

void readRawHDD(long LBA,char count,char* locationx){
        outportb(hdddevice.io_base+6, 0xE0 | (hdddevice.slave << 4) | ((LBA >> 24) & 0x0F));
        outportb(hdddevice.io_base+1, 0x00);
        outportb(hdddevice.io_base+2, (unsigned char) count);
        outportb(hdddevice.io_base+3, (unsigned char) LBA);
        outportb(hdddevice.io_base+4, (unsigned char)(LBA >> 8));
        outportb(hdddevice.io_base+5, (unsigned char)(LBA >> 16));
        outportb(hdddevice.io_base+7, 0x20);
        int status;
        while((status = inportb(hdddevice.io_base+7)) & 0x80 ){
                if((status >> 0) & 1){kernel_print("READERROR");for(;;);}
                asm volatile("pause");
        }
        int U = 0;
        int i = 0;
        for(i = 0 ; i < (512/2) ; i++){
                short X = inportw(hdddevice.io_base);
                char A = X;
                char B = (X >> 0x08);
                locationx[U++] = A;
                locationx[U++] = B;
        }
}

