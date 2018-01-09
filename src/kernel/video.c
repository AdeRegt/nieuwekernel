#include "sos.h"

void update_cursor(int x, int y){
        unsigned short pos = y * 80 + x;
        outportb(0x3D4, 0x0F);
        outportb(0x3D5, (unsigned char) (pos & 0xFF));
        outportb(0x3D4, 0x0E);
        outportb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

void setColor(char x){
        background = x;
}

void kernel_print(char* msg){
        char deze;
        int cnt = 0;
        while((deze = msg[cnt++])!=0x00){
                putc(deze);
        }
}


void putc(char deze){
        vidmempoint= (cury*160)+(curx*2);
        if(deze=='\n'){
                curx = 0;
                cury++;
        }else if(deze=='\t'){
                if(curx<10){
                        curx = 10;
                }else if(curx<20){
                        curx = 20;
                }else{
                        curx = 30;
                }
        }else{
                videomemory[vidmempoint++] = deze;
                videomemory[vidmempoint++] = background;
                curx++;
                if(curx==80){
                        curx = 0;
                        cury++;
                }
        }
}

char* itoa(int value, char* str, int base) {

    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if (value < 0 && base == 10) {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

void puti(int a){
        char* ss = "    ";
        itoa(a,ss,10);
        kernel_print(ss);
}

void puth(int a){
        putH(a&0x00ff);
        putH((a>>8)&0x00ff);
}

void putL(long a){
        unsigned char * p = (unsigned char*)&a;
        putH(p[3]);
        putH(p[2]);
        putH(p[1]);
        putH(p[0]);
}

void putN(char a){
        if(a==0x00){putc('0');}
        if(a==0x01){putc('1');}
        if(a==0x02){putc('2');}
        if(a==0x03){putc('3');}
        if(a==0x04){putc('4');}
        if(a==0x05){putc('5');}
        if(a==0x06){putc('6');}
        if(a==0x07){putc('7');}
        if(a==0x08){putc('8');}
        if(a==0x09){putc('9');}
        if(a==0x0a){putc('A');}
        if(a==0x0b){putc('B');}
        if(a==0x0c){putc('C');}
        if(a==0x0d){putc('D');}
        if(a==0x0e){putc('E');}
        if(a==0x0f){putc('F');}
}

void putH(char a){
        putN((a>>4)&0x000f);
        putN(a&0x000f);

}


void cls(){
        curx = 0;
        cury = 0;
        vidmempoint = 0;
        int i = 0;
        for(i = 0 ; i < 8000 ; i++){
                putc(' ');
        }
        curx = 0;
        cury = 0;
        vidmempoint = 0;
}

