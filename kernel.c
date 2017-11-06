typedef struct {
	unsigned int io_base;
	unsigned int control;
	unsigned int slave;
	unsigned long read;
	unsigned long write;
	unsigned long eject;
}ata_device;

typedef struct{
	unsigned char mountname[6];
	ata_device device;
	unsigned long loadfile;
	unsigned long writefile;
	unsigned long dir;
}FilesystemMountpoint;

FilesystemMountpoint mounts[10];
int mountcount = 0;
#define ATAPI_SECTOR_SIZE 2048

ata_device ata_primairy_master  = {.io_base = 0x1F0, .control = 0x3F6, .slave = 0};
ata_device ata_primairy_slave   = {.io_base = 0x1F0, .control = 0x3F6, .slave = 1};
ata_device ata_secondary_master = {.io_base = 0x170, .control = 0x376, .slave = 0};
ata_device ata_secondary_slave  = {.io_base = 0x170, .control = 0x376, .slave = 1};

char irq_ata_fired = 0;
//int cdromskiplength = 0;
ata_device cdromdevice;


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

void kernel_main();
void kernel_print(char* msg);
void putc(char deze);
void puti(int a);
void puth(int a);
inline unsigned char inportb(unsigned int port);
inline unsigned short inportw(unsigned int port);
inline void outportb(unsigned int port,unsigned char value);
inline void outportw(unsigned int port,unsigned short value);
void lidt();
void setInterrupt(int i, unsigned long base);
void keyboard_init();
void readRawCDROM(long lba,char count,char* locationx);
void ata_device_detect(ata_device dev);
void ata_init();
void initBootCDROM();
void dirBootCDROM();
int charstoint(char a,char b,char c,char d);
void registerMount(unsigned char mountname[6],ata_device device,unsigned long loadfile,unsigned long writefile,unsigned long dir);
void cls();
char getc();
void setColor(char x);
int selectDevice();
void kernelcall(long address);
void selectFile();

char filelistbuffer[500];
char selectedfile[100];

void kernel_main(){
	kernel_print("Loading...");
	mountcount = 0;
	direntcnt = 0;
	filentcnt = 0;
	cdromfloor = 1;
	lidt();
	keyboard_init();
	ata_init();
	initBootCDROM();
	FilesystemMountpoint mnt = mounts[selectDevice()];
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
			putc(' ');putc(mounts[i].mountname[0]);putc(mounts[i].mountname[1]);putc(mounts[i].mountname[2]);putc(mounts[i].mountname[3]);putc(mounts[i].mountname[4]);putc(mounts[i].mountname[5]);
			kernel_print("                                                                         ");
		}
		char x = getc();
		if(x=='\n'){break;}else if(x==1){s++;}else if(x==2){s--;}
	}
	return s;
}

void registerMount(unsigned char mountname[6],ata_device device,unsigned long loadfile,unsigned long writefile,unsigned long dir){
	int i = 0;
	for(i = 0 ; i < 6 ; i++){
		mounts[mountcount].mountname[i] = mountname[i];
	}
	mounts[mountcount].device = device;
	mounts[mountcount].loadfile = loadfile;
	mounts[mountcount].writefile = writefile;
	mounts[mountcount].dir = dir;
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
	for(i = 0 ; i < sizeof(filelistbuffer) ; i++){
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
	char* ss = "  ";
	itoa(a,ss,16);
	kernel_print(ss);
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
		registerMount((char*)"UNKNOWN",dev,0,0,0);
	}else if(atapi_device_init(dev)){
//		kernel_print(" CDR ");
		dev.read = (unsigned long)&readRawCDROM;
		cdromdevice = dev;
		registerMount((char*)"ATAPIbd",dev,&openFileBootCDROM,0,&dirBootCDROM);
	}else if(ata_device_init(dev)){
		registerMount((char*)"UNKNOWN",dev,0,0,0);
		//kernel_print(" HDD ");
	}else{
		registerMount((char*)"UNKNOWN",dev,0,0,0);
		//kernel_print(" ??? ");
	}
}

void ata_int_ready(){
	((char*)0x10000)[0]=0x00;
}

void ata_int_wait(){
	int i = 0;
	while(((char*)0x10000)[0]!=0x01){
		i++;//asm volatile("pause");
		//if((status >> 0) & 1){kernel_print("READERROR");for(;;);}
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
