#include "sos.h"
void mouse_write(unsigned char a_write){
        //Wait to be able to send a command
        mouse_wait(1);
        //Tell the mouse we are sending a command
        outportb(0x64, 0xD4);
        //Wait for the final part
        mouse_wait(1);
        //Finally write
        outportb(0x60, a_write);
}

unsigned char mouse_read(){
        //Get response from mouse
        mouse_wait(0);
        return inportb(0x60);
}

void mouse_wait(unsigned char type){
  unsigned int _time_out=100000;
  if(type==0){

    while(_time_out--) {
      if((inportb(0x64) & 1)==1){
        return;
      }
    }
    if(_time_out==0){kernel_print("Mouse not pressent. Press any key to continue");getc();}
    return;
  }
  else{
    while(_time_out--) {
      if((inportb(0x64) & 2)==0){
        return;
      }
    }
    if(_time_out==0){kernel_print("Mouse not pressent. Press any key to continue");getc();}
    return;
  }
}

int mousex = 0;
int mousey = 0;
void mouse_handler(){
  static unsigned char cycle = 0;
  static unsigned char mouse_bytes[3];
  mouse_bytes[cycle++] = inportb(0x60);

  if (cycle == 3) { // if we have all the 3 bytes...
    cycle = 0; // reset the counter
    // do what you wish with the bytes, this is just a sample
    if ((mouse_bytes[0] & 0x80) || (mouse_bytes[0] & 0x40))
      return; // the mouse only sends information about overflowing, do not care about it and return
    if (!(mouse_bytes[0] & 0x20)){
        if(mouse_bytes[2]==0xff){
        mousey += 1;
        }else if(mouse_bytes[2]==0x01){
        mousey -= 1;
        }
    }
      //mousey |= 0xFFFFFF00; //delta-y is a negative value
    if (!(mouse_bytes[0] & 0x10)){
        if(mouse_bytes[1]==0xff){
        mousex += 1;
        }else if(mouse_bytes[1]==0x01){
        mousex -= 1;
        }
    }
      //mousex |= 0xFFFFFF00; //delta-x is a negative value
    if (mouse_bytes[0] & 0x4)

      kernel_print("Middle button is pressed!n");
    if (mouse_bytes[0] & 0x2)
      kernel_print("Right button is pressed!n");
    if (mouse_bytes[0] & 0x1)
      kernel_print("Left button is pressed!n");
    // do what you want here, just replace the puts's to execute an action for each button
    // to use the coordinate data, use mouse_bytes[1] for delta-x, and mouse_bytes[2] for delta-y
    //if(mousex<0){mousex=1;}if(mousey<0){mousey=1;}
    //cls();
    if(mousex<0||mousex>50){mousex=20;}
    if(mousey<0||mousey>20){mousey=10;}
    curx = 0;cury = 0;
    putH(mouse_bytes[1]);putc(' ');putH(mouse_bytes[2]);putc(' ');putH(mousex);putc(' ');putH(mousey);putc(' ');
    update_cursor(mousex,mousey);
  }
}

void mouse_init(){
        mouse_wait(1);
        outportb(0x64,0xA8);
        mouse_wait(1);
        outportb(0x64,0x20);
        unsigned char status_byte;
        mouse_wait(0);
        status_byte = (inportb(0x60) | 2);
        mouse_wait(1);
        outportb(0x64, 0x60);
        mouse_wait(1);
        outportb(0x60, status_byte);
        mouse_write(0xF6);
        mouse_read();
        mouse_write(0xF4);
        mouse_read();
}

