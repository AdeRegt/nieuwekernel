#include "sos.h"
FilesystemMountpoint mounts[10];
unsigned int mountcount = 0;

ata_device ata_primairy_master  = {.io_base = 0x1F0, .control = 0x3F6, .slave = 0};
ata_device ata_primairy_slave   = {.io_base = 0x1F0, .control = 0x3F6, .slave = 1};
ata_device ata_secondary_master = {.io_base = 0x170, .control = 0x376, .slave = 0};
ata_device ata_secondary_slave  = {.io_base = 0x170, .control = 0x376, .slave = 1};

unsigned char irq_ata_fired = 0;
ata_device cdromdevice;
ata_device hdddevice;

DIRTABLEENTRY dirent[MAXDIRIDENT];
unsigned int direntcnt = 0;

FILETABLEENTRY filent[MAXDIRIDENT];
int filentcnt = 0;
unsigned char cdromfloor = 1;//1

MBRENTRY mbrt[4];

char filelistbuffer[500];
char selectedfile[100];
FilesystemMountpoint mnt;
char* videomemory = (char*) 0xb8000;
unsigned int vidmempoint = 0;
unsigned char background = 0x7f;
unsigned char curx = 0;
unsigned char cury = 0;

char* programloadseg = (char*) 0x1000;

void kernel_main(){
	kernel_print("Loading...");
	mountcount = 0;
	direntcnt = 0;
	filentcnt = 0;
	cdromfloor = 1;
	lidt();
	keyboard_init();
	kernel_print("Enable mouse? Y=yes, Other=no");
	if(getc()=='y'){
		mouse_init();
	}
	ata_init();
	serials_init();
	initTasking();
	mnt = mounts[selectDevice()];
	while(1){
		kernelcall(mnt.dir);
		selectFile();
		kernelcall(mnt.loadfile);
		if(selectedfile[0]=='~'){
			cls();
			ELFHEADER* headers = (ELFHEADER*)0x2000;
			ELFHEADER header = headers[0];
			if((header.e_ident[0]==ELFMAG0&&header.e_ident[1]==ELFMAG1&&header.e_ident[2]==ELFMAG2&&header.e_ident[3]==ELFMAG3)){
				long execlocation = loadExecutable((void *)0x2000);
				if(execlocation!=0){
					kernelcall(execlocation);
				}else{
					kernel_print("Unable to execute program!\n");
				}
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
			kernel_print("Program finished!\n");
			getc();
			asm volatile("jmp kernel_main");
		}
	}
	for(;;);
}

//
// G L O B A L   F I L E S Y S T E M   C O N T R O L L
//
//

void selectFile(){
	unsigned int s = 0;
	while(1){
		setColor(0x7F);
		cls();
		setColor(0x52);
		kernel_print(" Select a file                                Sanderslando Operating System 2.0 ");
		unsigned int i = 0;
		int e = 0;
		int fo = 0;
		while(1){
			setColor((unsigned int)i==(unsigned int)s?0x5f:0x7F);
			putc(' ');
			int z = 0;
			int t = 0;
			while(1){
				char deze = filelistbuffer[e++];
				if(deze==0x00){fo=1;break;}
				if(deze==';'){break;}
				putc(deze);
				if((unsigned int)i==(unsigned int)s){
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
	unsigned int s = 0;
	while(1){
		setColor(0x7F);
		cls();
		setColor(0x52);
		kernel_print(" Select a device                              Sanderslando Operating System 2.0 ");
		unsigned int i = 0;
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
// S F S   F I L E S Y S T E M   H A N D L E R
//
//

void openFileSFS(){

}

void dirSFS(){

}

//
// F A T 3 2   F I L E S Y S T E M   H A N D L E R
//
//

typedef struct{
	unsigned char filename[8];
	unsigned char extension[3];
	unsigned char attributes;
	unsigned char caseinfo;
	unsigned char createtime;
	unsigned short hourandminute;
	unsigned short yearmontday;
	unsigned short lastaccesday;
	unsigned short highbytes;
	unsigned short lastmodifiedtime;
	unsigned short lastmodifieddate;
	unsigned short lowbytes;
	unsigned long size;
}FATTableEntry;



void openFileFAT(){}

void dirFAT(){
	char* buffer = (char*) 0x2000;
	unsigned long Partition_LBA_Begin = mbrt[mnt.partitionselect].lba;
	readRawHDD(Partition_LBA_Begin,1,buffer);
	unsigned short Number_of_Reserved_Sectors = ((short*)0x200E)[0];
	unsigned char Number_of_FATs = buffer[0x10];
	unsigned long Sectors_Per_FAT = ((long*)0x2024)[0];
//	unsigned long fat_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors;
	unsigned long cluster_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
	char* buf2 = (char*) 0x2000;
	readRawHDD(cluster_begin_lba,1,buf2);
	int cluster = 11;
	int cntx = 0;
	while(cluster<512){
		cntx++;
		if(cntx==64){
			cntx = 0;
			putc('X');
		}
		putc(buf2[cluster++]);
	}
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
			unsigned int i = 0;
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
	unsigned int i = 0;
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
                 unsigned int oxaA = buffer[gdx++];
                 unsigned int oxaB = buffer[gdx++];
                 unsigned int oxaC = buffer[gdx++];
                 unsigned int oxaD = buffer[gdx++];
                 dirent[direntcnt].ExtLBA = charstoint(oxaA,oxaB,oxaC,oxaD);
                 unsigned int oxaE = buffer[gdx++];
                 unsigned int oxaF = buffer[gdx++];
                 dirent[direntcnt].ParntDir = charstoint(0,0,oxaE,oxaF);
                 unsigned int eA = 0;
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
