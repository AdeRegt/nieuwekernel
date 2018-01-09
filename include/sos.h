#define NULL 0

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
	unsigned int partitionselect;
}FilesystemMountpoint;

#define ATAPI_SECTOR_SIZE 2048

typedef struct{
	unsigned int port;
}COMPort;

#define MAXSIZEFILENAME 30
#define MAXDIRIDENT 100
#define MAXPATHSIZE 120

typedef struct{
        unsigned int LengthOfDirIdent;
        unsigned int ExtAttrLngth;
        unsigned int ExtLBA;
        unsigned int ParntDir;
        unsigned char dirident[MAXSIZEFILENAME];
        unsigned int padding;
}DIRTABLEENTRY;

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

typedef struct{
	unsigned char crap[16-8];
	unsigned long lba;
	unsigned long size;
}MBRENTRY;

typedef struct{
	unsigned char filename[12];
	unsigned char attributes; // 0x00 deletedfile | 0x01 normal file | 0x02 directory
	unsigned long LBA;
	unsigned long sectorcount;
	unsigned long sizeinlastsector;
}SFSDirectoryEntry;

typedef struct{
	unsigned long parentfile;
	SFSDirectoryEntry ent[10];
}SFSDirectory;

typedef struct {
    unsigned long eax;
    unsigned long ebx;
    unsigned long ecx;
    unsigned long edx;
    unsigned long esi;
    unsigned long edi;
    unsigned long esp;
    unsigned long ebp;
    unsigned long eip;
    unsigned long eflags;
    unsigned long cr3;
} Registers;

typedef struct {
    Registers regs;
    struct Task *next;
} Task;


typedef unsigned long Elf32_Word;
typedef unsigned long Elf32_Addr;
typedef unsigned long Elf32_Off;
typedef unsigned long Elf32_Sword;
typedef unsigned short Elf32_Half;
#define ELFMAG0   0x7f
#define ELFMAG1   'E'
#define ELFMAG2   'L'
#define ELFMAG3   'F'
#define EI_NIDENT 16

typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half    e_type;
	Elf32_Half    e_machine;
	Elf32_Word    e_version;
	Elf32_Addr    e_entry;
	Elf32_Off     e_phoff;
	Elf32_Off     e_shoff;
	Elf32_Word    e_flags;
	Elf32_Half    e_ehsize;
	Elf32_Half    e_phentsize;
	Elf32_Half    e_phnum;
	Elf32_Half    e_shentsize;
	Elf32_Half    e_shnum;
	Elf32_Half    e_shstrndx;
} ELFHEADER;

typedef struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} ELFSECTION;

typedef struct {
	unsigned long st_name;
	unsigned long st_value;
	unsigned long st_size;
	unsigned char st_info;
	unsigned char st_other;
	unsigned short st_shndx;
}ELFSYMBOL;

typedef struct {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
} ELFRELOCATION;


#define SHT_NONE     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_NOBITS   8
#define SHT_REL 9

#define STT_SECTION 3

#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_ST_TYPE(i) ((i) & 0xf)
#define ELF32_R_SYM(i) ((i) >> 8)

void kernel_main();
void kernel_print(char* msg);
void putc(char deze);
void puti(int a);
void puth(int a);
void putH(char a);
void putN(char a);
void putL(long a);
unsigned char inportb(unsigned int port);
unsigned short inportw(unsigned int port);
void outportb(unsigned int port,unsigned char value);
void outportw(unsigned int port,unsigned short value);
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
void mouse_wait(unsigned char type);
void serials_init();
void init_serial(COMPort port);
char read_serial(COMPort port);
void write_serial(COMPort port,char a);
void initTasking();
void yield();
void createTask(Task *task, void (*main)(), unsigned long flags, unsigned long *pagedir);
int cmpstr(char* bufferA,char* bufferB,int size);
unsigned int strlen(char* buffer);
unsigned long loadExecutable(void * imagelocation);
