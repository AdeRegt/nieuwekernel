#include "sos.h"

//
// K E Y B O A R D   M A N A G E R
// http://wiki.osdev.org/PS/2_Keyboard
//

unsigned char kbdus[128] ={
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
  '9', '0', '-', '=', '\b',     /* Backspace */
  '\t',                 /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,                  /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
 '\'', '`',   0,                /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
  'm', ',', '.', '/',   0,                              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    2,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    1,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0xbe,       /* F12 Key */
    0,  /* All other keys are undefined */
};

char kbdswit = 0x00;


extern char kbdxx;
extern char pgetc();


char kbd_int(){
        unsigned int x = inportb(0x60);
        if(x<sizeof(kbdus)){
                kbdxx = x;
        }
        return 0;
}


char getc(){
        int deze = pgetc();
        //putc(kbdus[deze]);
        return kbdus[deze];
}

void keyboard_send_cmd(char val){
        outportb(0x60,val);
        while(inportb(0x60)!=0xFA){}
}

void keyboard_init(){
        // kijken of keyboard bestaat
        outportb(0x60,0xEE);
        int timeout = 0;
        while(inportb(0x60)!=0xEE){
                timeout++;
                if(timeout==0x10000){
                        kernel_print("FATAL: NO KEYBOARD");for(;;);
                }
        }
        outportb(0x60,0xFF);
        timeout = 0;
        while(inportb(0x60)!=0xAA){
                timeout++;
                if(timeout==0x10000){
                        kernel_print("FATAL: NO KEYBOARD");for(;;);
                }
        }
        keyboard_send_cmd(0xf6);
        keyboard_send_cmd(0xf4);
}

