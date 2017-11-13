typedef struct {
	unsigned int io_base;
	unsigned int control;
	unsigned int slave;
	unsigned long read;
	unsigned long write;
	unsigned long eject;
}ata_device;

typedef struct{
	unsigned char mountname[7];
	ata_device device;
	unsigned long loadfile;
	unsigned long writefile;
	unsigned long dir;
	int partitionselect;
}FilesystemMountpoint;

FilesystemMountpoint mounts[10];
int mountcount = 0;
#define ATAPI_SECTOR_SIZE 2048

ata_device ata_primairy_master  = {.io_base = 0x1F0, .control = 0x3F6, .slave = 0};
ata_device ata_primairy_slave   = {.io_base = 0x1F0, .control = 0x3F6, .slave = 1};
ata_device ata_secondary_master = {.io_base = 0x170, .control = 0x376, .slave = 0};
ata_device ata_secondary_slave  = {.io_base = 0x170, .control = 0x376, .slave = 1};

typedef struct{
	unsigned int port;
}COMPort;

COMPort port1 = {.port = 0x3f8};
COMPort port2 = {.port = 0x2f8};
COMPort port3 = {.port = 0x3e8};
COMPort port4 = {.port = 0x2e8};

char irq_ata_fired = 0;
//int cdromskiplength = 0;
ata_device cdromdevice;
ata_device hdddevice;


#define MAXSIZEFILENAME 30
#define MAXDIRIDENT 100
#define MAXPATHSIZE 120

typedef struct{
        int LengthOfDirIdent;
        int ExtAttrLngth;
        int ExtLBA;
        int ParntDir;
        char dirident[MAXSIZEFILENAME];
        int padding;
}DIRTABLEENTRY;

DIRTABLEENTRY dirent[MAXDIRIDENT];
int direntcnt = 0;

typedef struct{
        unsigned char lengthofdirrecord;
        unsigned char extendedattributereconrdlength;
        unsigned char lba1;
        unsigned char lba2;
        unsigned char lba3;
        unsigned char lba4;
        unsigned char lba5;
        unsigned char lba6;
        unsigned char lba7;
        unsigned char lba8;
        unsigned char len1;
        unsigned char len2;
        unsigned char len3;
        unsigned char len4;
        unsigned char len5;
        unsigned char len6;
        unsigned char len7;
        unsigned char len8;
        unsigned char datum[7];
        unsigned char flags;
        unsigned char fileunitsize;
        unsigned char interleavegap;
        unsigned long vsn;
        unsigned char filenamelength;
        unsigned char filename[MAXSIZEFILENAME];
}FILETABLEENTRY;

FILETABLEENTRY filent[MAXDIRIDENT];
int filentcnt = 0;

char cdromfloor = 1;//1


typedef struct{
char    A[1];// 0x7F
char    B[3];// E L F
char    bits;//1 = 32 | 2 = 64
char    endian;//1 = little | 2 = big
char    ver1;
char    target;
char    abi;
char    unused[7];
short   type;
short   machine;
long    version;
long    entrypoint;
long    programheader;
long    segmentheaders;
long    flags;
short   headersize;
short   sizeprogramheader;
short   entriesprogramheader;
short   sizesectionheader;
short   entriessectionheader;
short   sectionnamelist;
}ELFHEADER;

typedef struct{
long name;
long type;
long flags;
long addr;
long offset;
long size;
long link;
long info;
long adralign;
long entsize;
}ELFSECTION;

typedef struct{
char crap[16-8];
long lba;
long size;
}MBRENTRY;
MBRENTRY mbrt[4];

void kernel_main();
void kernel_print(char* msg);
void putc(char deze);
void puti(int a);
void puth(int a);
void putH(char a);
void putN(char a);
inline unsigned char inportb(unsigned int port);
inline unsigned short inportw(unsigned int port);
inline void outportb(unsigned int port,unsigned char value);
inline void outportw(unsigned int port,unsigned short value);
void lidt();
void setInterrupt(int i, unsigned long base);
void keyboard_init();
void readRawCDROM(long lba,char count,char* locationx);
void readRawHDD(long LBA,char count,char* locationx);
void ata_device_detect(ata_device dev);
void ata_init();
void initBootCDROM();
void dirBootCDROM();
void openFileBootCDROM();
void dirFAT();
void openFileFAT();
int charstoint(char a,char b,char c,char d);
void registerMount(unsigned char mountname[6],ata_device device,unsigned long loadfile,unsigned long writefile,unsigned long dir,int partitionselect);
void cls();
char getc();
void setColor(char x);
int selectDevice();
void kernelcall(long address);
void selectFile();
void mouse_init();
void serials_init();
void init_serial(COMPort port);
char read_serial(COMPort port);
void write_serial(COMPort port,char a);

char filelistbuffer[500];
char selectedfile[100];
FilesystemMountpoint mnt;

void kernel_main(){
	kernel_print("Loading...");
	mountcount = 0;
	direntcnt = 0;
	filentcnt = 0;
	cdromfloor = 1;
	lidt();
	keyboard_init();
	mouse_init();
	ata_init();
	serials_init();
	mnt = mounts[selectDevice()];
	while(1){
		kernelcall(mnt.dir);
		selectFile();
		kernelcall(mnt.loadfile);
		if(selectedfile[0]=='~'){
			cls();
			ELFHEADER* headers = (ELFHEADER*)0x2000;
			ELFHEADER header = headers[0];
			if((header.A[0]==0x7F&&header.B[0]=='E'&&header.B[1]=='L'&&header.B[2]=='F')){
				ELFSECTION* progsect = (ELFSECTION*)(0x2000+header.segmentheaders);
				int i = 0;
				for(i = 0 ; i < header.entriessectionheader ; i++){
				        if(progsect[i].addr!=0){
				                int z = 0;
				                char* buffer = (char*)progsect[i].addr;
				                for(z = 0 ; z < progsect[i].size ; z++){
				                        buffer[z] = ((char*)progsect[i].offset+0x2000)[z];
				                }
				        }
                		}
                		kernelcall(header.entrypoint);
			}else{
				kernel_print("[R]un, [D]isplay or Cancel?");
				int i = 0;
				char sel = getc();
				cls();
				if(sel=='r'){
					kernelcall(0x2000);
				}else if(sel=='d'){
					for(i = 0 ; i < 1000 ; i++){
						putc(((char*)0x2000)[i]);
					}
				}
			}
			getc();
			asm volatile("jmp kernel_main");
		}
	}
	for(;;);
}


//
// S E R I A L   P O R T   C O N T R O L L 
//
//

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


//
// G L O B A L   F I L E S Y S T E M   C O N T R O L L 
//
//

void selectFile(){
	int s = 0;
	while(1){
		setColor(0x7F);
		cls();
		setColor(0x52);
		kernel_print(" Select a file                                Sanderslando Operating System 2.0 ");
		int i = 0;
		int e = 0;
		int fo = 0;
		while(1){
			setColor(i==s?0x5f:0x7F);
			putc(' ');
			int z = 0;
			int t = 0;
			while(1){
				char deze = filelistbuffer[e++];
				if(deze==0x00){fo=1;break;}
				if(deze==';'){break;}
				putc(deze);
				if(i==s){
					selectedfile[t++] = deze;
					selectedfile[t] = 0x00;
				}
				z++;
			}
			int q = 0;
			for(q = z ; q  < 79 ; q++){
				putc(' ');
			}
			i++;
			if(fo){break;}
		}
		char x = getc();
		if(x=='\n'){break;}else if(x==1){s++;}else if(x==2){s--;}
	}
}

void kernelcall(long address){ 
	asm volatile ("call %0": : "r"(address));
}


int selectDevice(){
	int s = 0;
	while(1){
		setColor(0x7F);
		cls();
		setColor(0x52);
		kernel_print(" Select a device                              Sanderslando Operating System 2.0 ");
		int i = 0;
		for( i = 0 ; i < mountcount ; i++){
			setColor(i==s?0x5f:0x7F);
			putc(' ');putc(mounts[i].mountname[0]);putc(mounts[i].mountname[1]);putc(mounts[i].mountname[2]);putc(mounts[i].mountname[3]);putc(mounts[i].mountname[4]);putc(mounts[i].mountname[5]);putc(mounts[i].mountname[6]);
			kernel_print("                                                                        ");
		}
		char x = getc();
		if(x=='\n'){break;}else if(x==1){s++;}else if(x==2){s--;}
	}
	return s;
}

void registerMount(unsigned char mountname[6],ata_device device,unsigned long loadfile,unsigned long writefile,unsigned long dir,int partitionselect){
	int i = 0;
	for(i = 0 ; i < 6 ; i++){
		mounts[mountcount].mountname[i] = mountname[i];
	}
	mounts[mountcount].device = device;
	mounts[mountcount].loadfile = loadfile;
	mounts[mountcount].writefile = writefile;
	mounts[mountcount].dir = dir;
	mounts[mountcount].partitionselect = partitionselect;
	mountcount++;
}

int charstoint(char a,char b,char c,char d){
        unsigned int final = 0;
        final |= ( a << 24 );
        final |= ( b << 16 );
        final |= ( c <<  8 );
        final |= ( d       );
        return final;
}

//
// F A T 3 2   F I L E S Y S T E M   H A N D L E R
//
//

void openFileFAT(){}

void dirFAT(){
	char* buffer = (char*) 0x2000;
	readRawHDD(mbrt[mnt.partitionselect].lba,1,buffer);
	short locroot = (buffer[14]<<8)|buffer[15];
//	puth(locroot);putc(' ');
	readRawHDD(mbrt[mnt.partitionselect].lba+32,1,buffer);
	int i = 0;
	for(i = 0 ; i < 512 ; i++){putH(buffer[i]);putc(' ');}
	getc();
}

//
// I S O   F I L E S Y S T E M   H A N D L E R
//
//

void openFileBootCDROM(){
	if(selectedfile[0]=='*'||selectedfile[0]=='.'){
		if(selectedfile[1]==0x00||(selectedfile[0]=='.'&&selectedfile[1]==0x00)){
			DIRTABLEENTRY E = dirent[cdromfloor-1];
			cdromfloor = E.ParntDir;
		}else{
			int i = 0;
			for( i = 0 ; i < direntcnt ; i++){
				DIRTABLEENTRY E = dirent[i];
				int z = 0;
				while(1){
					char A = E.dirident[z];
					char B = selectedfile[1+z];
					z++;
					if(B==0x00){
						break;
					}
					if(A!=B){goto notmine;}
				}
				cdromfloor = i+1;
				notmine:
				continue;
			}
		}
	}else if(selectedfile[0]=='~'){
		int i = 0;
		for(i = 0 ; i < filentcnt ; i++){
			int z = 0;
			for(z = 0 ; z < (filent[i].filenamelength)-2 ; z++){
				if(filent[i].filename[z]!=selectedfile[z+1]){
					goto nope;
				}
			}
			char* buffer = (char*) 0x2000;
			unsigned long int l = filent[i].lba8 | (filent[i].lba7 << 8) | (filent[i].lba6 << 16) | (filent[i].lba5 << 24);
			unsigned long int j = filent[i].len8 | (filent[i].len7 << 8) | (filent[i].len6 << 16) | (filent[i].len5 << 24);
			readRawCDROM(l,(j/ATAPI_SECTOR_SIZE)+1,buffer);
			return;
			nope:
			z = 0;
		}
	}
}

void dirBootCDROM(){
	int selector = 0;
	char* buffer = (char*) 0x2000;
	signed int i = 0;
	for(i = 0 ; i < (signed int)sizeof(filelistbuffer) ; i++){
		filelistbuffer[i] = 0x00;
	}
	filelistbuffer[selector++] = '.';
	filelistbuffer[selector++] = ';';
	for(i = 0 ; i < direntcnt ; i++){
		DIRTABLEENTRY E = dirent[i];
		if(cdromfloor==E.ParntDir){
			int y = 0;
			char this;
			filelistbuffer[selector++] = '*';
			while((this=E.dirident[y++])!=0x00){
			//for(y = 0 ; y < E.LengthOfDirIdent; y++){
				filelistbuffer[selector++] = this;
			}
			filelistbuffer[selector++] = ';';
		}
	}
	readRawCDROM(dirent[cdromfloor-1].ExtLBA,1,buffer);
	i = 0;
	filentcnt = 0;
	while(1){
		
		if(buffer[i]==';'&&buffer[i+1]=='1'){
			// we gaan eerst proberen om de volledige tekst te krijgen en dan alles kopieren naar de file
			char sucnt = 1;
			int intern = 0;
			int y = 0;
			while(buffer[i-(y++)]!=sucnt){
				sucnt++;
			}
			sucnt--;
			sucnt--;
			intern += sucnt;
			intern += 1;
			intern += 32;
			int t = 0;
			for(t = 0 ; t < (33+sucnt) ; t++){
				((char*)&filent[filentcnt])[t] = buffer[(i-intern)+t];
			}
			filelistbuffer[selector++] = '~';
			//filent[filentcnt].filename[filent[filentcnt].filenamelength] = 0x00;
			for(t = 0 ; t < (filent[filentcnt].filenamelength)-2 ; t++){
				char deze = filent[filentcnt].filename[t];
				filelistbuffer[selector++] = deze;
				//putc(deze);
			}
			//puth(filent[filentcnt].filenamelength);
			//getc();
			filelistbuffer[selector++] = ';';
			filentcnt++;
		}
		i++;
		if(i==20000){
			break;
		}
	}
	filelistbuffer[selector++] = 0x00;
	
}

void initBootCDROM(){
	//
	// F I N D   L O G I C A L  P R I M A I R Y   B L O C K
	//
	int sectorprimairydescriptor = 0;
	int i = 0 ;
	char* buffer = (char*) 0x2000;
	for(i = 0 ; i < 100 ; i++){
		readRawCDROM(i,1,buffer);
		if(buffer[0]==0x01&&buffer[1]=='C'&&buffer[2]=='D'&&buffer[3]=='0'&&buffer[4]=='0'&&buffer[5]=='1'){
			goto foundsuperblock;
		}
		sectorprimairydescriptor++;
	}
	kernel_print("FATAL: Unable to find primairy block!");for(;;);
	foundsuperblock:
	readRawCDROM(sectorprimairydescriptor,1,buffer);
	int dt = charstoint(buffer[148],buffer[149],buffer[150],buffer[151]);
	readRawCDROM(dt,1,buffer);
	int gdx = 0;
        //int ptx = 0;
        while(1){
                 dirent[direntcnt].LengthOfDirIdent = buffer[gdx++];
                 dirent[direntcnt].ExtAttrLngth = buffer[gdx++];
                 int oxaA = buffer[gdx++];
                 int oxaB = buffer[gdx++];
                 int oxaC = buffer[gdx++];
                 int oxaD = buffer[gdx++];
                 dirent[direntcnt].ExtLBA = charstoint(oxaA,oxaB,oxaC,oxaD);
                 int oxaE = buffer[gdx++];
                 int oxaF = buffer[gdx++];
                 dirent[direntcnt].ParntDir = charstoint(0,0,oxaE,oxaF);
                 int eA = 0;
                 for(eA = 0 ; eA < dirent[direntcnt].LengthOfDirIdent ; eA++){
                 	dirent[direntcnt].dirident[eA] = buffer[gdx++];
                 }
                 if(dirent[direntcnt].LengthOfDirIdent==0x00){
                        break;
                 }
                 dirent[direntcnt].padding = buffer[gdx++];
                 if(dirent[direntcnt].padding!=0x00){
                        gdx--;
                 }
		direntcnt++;
          }
	//kernel_print(" OK ");
}

//
// M O U S E   D R I V E R
//
//

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
  static char mouse_bytes[3];
  mouse_bytes[cycle++] = inportb(0x60);
 
  if (cycle == 3) { // if we have all the 3 bytes...
    cycle = 0; // reset the counter
    // do what you wish with the bytes, this is just a sample
    if ((mouse_bytes[0] & 0x80) || (mouse_bytes[0] & 0x40))
      return; // the mouse only sends information about overflowing, do not care about it and return
    if (!(mouse_bytes[0] & 0x20))
      mousey |= 0xFFFFFF00; //delta-y is a negative value
    if (!(mouse_bytes[0] & 0x10))
      mousex |= 0xFFFFFF00; //delta-x is a negative value
    if (mouse_bytes[0] & 0x4)
      kernel_print("Middle button is pressed!n");
    if (mouse_bytes[0] & 0x2)
      kernel_print("Right button is pressed!n");
    if (mouse_bytes[0] & 0x1)
      kernel_print("Left button is pressed!n");
    // do what you want here, just replace the puts's to execute an action for each button
    // to use the coordinate data, use mouse_bytes[1] for delta-x, and mouse_bytes[2] for delta-y
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

//
// V I D E O   M A N A G E R
//


char* videomemory = (char*) 0xb8000;
int vidmempoint = 0;
char background = 0x7f;
char curx = 0;
char cury = 0;

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
	putN(a&0x000f);
	putN((a>>4)&0x000f);
	
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
	char* buffer = (char*) 0x2000;
	readRawHDD(mbrt[selector].lba,1,buffer);
	if(buffer[66]==0x28||buffer[66]==0x29){
		registerMount((unsigned char*)"HDDFAT0",dev,(unsigned long)&openFileFAT,0,(unsigned long)&dirFAT,selector);
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
//		kernel_print(" CDR ");
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
		if(mbrt[0].lba!=0){
			detectFilesystem(dev,0);
		}
		if(mbrt[1].lba!=0){
			detectFilesystem(dev,1);
		}
		if(mbrt[2].lba!=0){
			detectFilesystem(dev,2);
		}
		if(mbrt[3].lba!=0){
			detectFilesystem(dev,3);
		}
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
	//	inportb(cdromdevice.io_base+0x206);
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
//		putc(A);putc(B);
	}
	//ata_int_wait();
	//while((status = inportb(cdromdevice.io_base+7)) & 0x88 ){asm volatile("pause"); }

}

//
// K E Y B O A R D   M A N A G E R
// http://wiki.osdev.org/PS/2_Keyboard
//

unsigned char kbdus[128] ={
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    2,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    1,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0xbe,	/* F12 Key */
    0,	/* All other keys are undefined */
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

//
// P O R T   H A N D E L I N G
//
//
// --------------------------------------------------------------------------
// From NOPE-OS
// --------------------------------------------------------------------------
// arminb@aundb-online.de
// --------------------------------------------------------------------------


/* Input a byte from a port */
inline unsigned char inportb(unsigned int port){
   unsigned char ret;
   asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
   return ret;
}

/* Input a word from a port */
inline unsigned short inportw(unsigned int port){
	unsigned short ret;
	asm volatile ("inw %%dx,%%ax":"=a"(ret):"d"(port));
//	asm volatile ("inw %1, %0" : "=a" (rv) : "dN"(port));
	return ret;
}

inline unsigned long inportd(unsigned int port){
	unsigned long ret;
	asm volatile ("ind %%dx,%%eax":"=a"(ret):"d"(port));
	return ret;
}
/* Output a byte to a port */
/* July 6, 2001 added space between :: to make code compatible with gpp */

inline void outportb(unsigned int port,unsigned char value){
   asm volatile ("outb %%al,%%dx": :"d" (port), "a" (value));
}

inline void outportw(unsigned int port,unsigned short value){
	asm volatile("outw %%ax,%%dx": :"d"(port),"a"(value));
}

//
// -------------------------------------------
// END FROM NOPE OS
//
