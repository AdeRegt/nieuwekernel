#include "sos.h"
//
// C D R O M   M A N A G E R
//
//

char read_cmd[12] = {0xA8,0,0,0,0,0,0,0,0,0,0,0};
short* readw = (short*) &read_cmd;

void readRawCDROM(long lba,char count,char* locationx){//E
        outportb(cdromdevice.io_base+6,0xE0|(cdromdevice.slave <<4) | ((lba >> 24) & 0x0f));//cdromdevice.slave & ( 1 << 4 ));
        // wachtend
        //int u = 0;
        //for(u = 0 ; u < 5 ; u++){
        //      inportb(cdromdevice.io_base+0x206);
        //}
        outportb(cdromdevice.io_base+1,0x00);
        outportb(cdromdevice.io_base+4,ATAPI_SECTOR_SIZE & 0xFF );
        outportb(cdromdevice.io_base+5,ATAPI_SECTOR_SIZE >> 8);
        outportb(cdromdevice.io_base+7,0xA0);
        // POLL UNTILL UNBUSSY
        char status;
        while((status = inportb(cdromdevice.io_base+7)) & 0x80 ){
                if((status >> 0) & 1){kernel_print("READERROR");for(;;);}
                asm volatile("pause");
        }
        if((status >> 0) & 1){kernel_print("READERROR");for(;;);}
        while(!((status = inportb(cdromdevice.io_base+7))&0x8) && !(status & 0x1)){
                if((status >> 0) & 1){kernel_print("READERROR");for(;;);}
                asm volatile("pause");
        }
        if((status >> 0) & 1){kernel_print("READERROR");for(;;);}
        if(status & 0x1){
                kernel_print("FATAL ERROR: status & 0x1 ");for(;;);
        }
        ata_int_ready();
        read_cmd[9] = count;
        read_cmd[2] = (lba >> 0x18) & 0xFF;
        read_cmd[3] = (lba >> 0x10) & 0xFF;
        read_cmd[4] = (lba >> 0x08) & 0xFF;
        read_cmd[5] = (lba >> 0x00) & 0xFF;
        int y = 0;
        for(y = 0 ; y < 6 ; y++){
                outportw(cdromdevice.io_base,readw[y]);
        }
        //int cnt = 0;
        ata_int_wait();
        int size = (((int)inportb(cdromdevice.io_base+5))<<8)|(int)(inportb(cdromdevice.io_base+4));
        if(size!=ATAPI_SECTOR_SIZE){kernel_print("FATAL ERROR size!=ATAPI_SECTOR_SIZE");for(;;);}
        //short *readev = (short*) locationx;//0x2000;
        int i = 0;
        //ata_int_ready();
        int U = 0;
        while((status = inportb(cdromdevice.io_base+7)) & 0x80 ){
                if((status >> 0) & 1){kernel_print("READERROR");for(;;);}
                asm volatile("pause");
        }

        for(i = 0 ; i < (ATAPI_SECTOR_SIZE/2)*count ; i++){
                short X = inportw(cdromdevice.io_base);
                char A = X;//(X >> 0x00) & 0xFF;
                char B = (X >> 0x08); //& 0xFF;
                locationx[U++] = A;
                locationx[U++] = B;
//              putc(A);putc(B);
        }
        //ata_int_wait();
        //while((status = inportb(cdromdevice.io_base+7)) & 0x88 ){asm volatile("pause"); }

}


