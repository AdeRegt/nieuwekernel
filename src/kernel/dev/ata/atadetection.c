#include "sos.h"
//
// A T A   I N F O
// https://github.com/mallardtheduck/osdev/blob/master/src/modules/ata/ata.cpp
//


void ata_init(){
        ata_device_detect(ata_primairy_master);
        ata_device_detect(ata_primairy_slave);
        ata_device_detect(ata_secondary_master);
        ata_device_detect(ata_secondary_slave);
}

void ata_wait(ata_device dev){
        inportb(dev.control);
        inportb(dev.control);
        inportb(dev.control);
        inportb(dev.control);
}

void ata_wait2(ata_device dev){
        ata_wait(dev);
        char status = 0;
        while((status=inportb(dev.io_base+0x07))&0x80){}
}

int ata_device_init(ata_device dev){
        outportb(dev.io_base+0x01,1);
        outportb(dev.control,0);
        outportb(dev.io_base+0x06,0xA0 | dev.slave <<4 );
        ata_wait(dev);
        outportb(dev.io_base+0x07,0xEC);
        ata_wait(dev);
        int status = inportb(dev.io_base+0x07);
        if(status==0||(status & 0x01 )){
                return 0;
        }
        ata_wait2(dev);
        int i = 0;
        for(i = 0 ; i < 256 ; i++){
                inportb(dev.io_base);
        }
        outportb(dev.control,0);
        return 1;
}

int atapi_device_init(ata_device dev){
        outportb(dev.io_base+0x01,1);
        outportb(dev.control,0);
        outportb(dev.io_base+0x06,0xA0 | dev.slave <<4 );
        ata_wait(dev);
        outportb(dev.io_base+0x07,0xEC);
        ata_wait(dev);
        unsigned char cl = inportb(dev.io_base+0x04);
        unsigned char ch = inportb(dev.io_base+0x05);
        if(!(cl == 0x14 && ch == 0xEB) && !(cl == 0x69 && ch == 0x96)){
                return 0;
        }
        return 1;
}

void detectFilesystem(ata_device dev,int selector){
        if((mbrt[selector].crap[4])==0x0C){
                registerMount((unsigned char*)"HDDFAT0",dev,(unsigned long)&openFileFAT,0,(unsigned long)&dirFAT,selector);
        }else if((mbrt[selector].crap[4])==0x83){
                registerMount((unsigned char*)"HDDEXT0",dev,(unsigned long)&openFileFAT,0,(unsigned long)&dirFAT,selector);
        }else if((mbrt[selector].crap[4])==0x19){
                registerMount((unsigned char*)"HDDSFS0",dev,(unsigned long)&openFileSFS,0,(unsigned long)&dirSFS,selector);
        }else if((mbrt[selector].crap[4])==0x00){
                registerMount((unsigned char*)"EMPTY  ",dev,(unsigned long)&openFileSFS,0,(unsigned long)&dirSFS,selector);
        }else{
                registerMount((unsigned char*)"UNKNOWN",dev,(unsigned long)&openFileSFS,0,(unsigned long)&dirSFS,selector);
        }
}

// void registerMount(unsigned char mountname[6],ata_device device,unsigned long loadfile,unsigned long writefile,unsigned long dir)

void ata_device_detect(ata_device dev){
        // first, softreset
        outportb(dev.control,0x04);
        outportb(dev.control,0x00);

        outportb(dev.io_base + 0x06 , 0xA0 | dev.slave << 4 );

        ata_wait(dev);

        unsigned char cl = inportb(dev.io_base + 0x04);
        unsigned char ch = inportb(dev.io_base + 0x05);

        if(cl==0xFF&&ch==0xFF){
                //kernel_print(" NUL ");
        }else if(atapi_device_init(dev)){
//              kernel_print(" CDR ");
                dev.read = (unsigned long)&readRawCDROM;
                cdromdevice = dev;
                registerMount((unsigned char*)"CDROM01",dev,(unsigned long)&openFileBootCDROM,0,(unsigned long)&dirBootCDROM,0);
                initBootCDROM();
        }else if(ata_device_init(dev)){
                hdddevice = dev;
                char* buffer = (char*) 0x2000;
                readRawHDD(0,1,buffer);
                char* localtarget = (char*)&mbrt;
                int i = 0;

                for(i = 0 ; i < (signed int)(sizeof(MBRENTRY)*4) ; i++){
                        localtarget[i] = buffer[446+i];
                }
                detectFilesystem(dev,0);
                detectFilesystem(dev,1);
                detectFilesystem(dev,2);
                detectFilesystem(dev,3);
                //registerMount((char*)"HARDISK",dev,0,0,0);
        }else{
                //kernel_print(" ??? ");
        }
}

void ata_int_ready(){
        ((char*)0x10000)[0]=0x00;
}

void ata_int_wait(){
        int i = 0;
        while(((char*)0x10000)[0]!=0x01){
                i++;
        }
}


