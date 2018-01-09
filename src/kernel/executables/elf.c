#include "sos.h"

unsigned long loadExecutable(void * imagelocation){
//	ELFHEADER header = ((ELFHEADER*)imagelocation)[0];
	ELFHEADER * header = (ELFHEADER *)imagelocation;
	if((header->e_ident[0]==ELFMAG0&&header->e_ident[1]==ELFMAG1&&header->e_ident[2]==ELFMAG2&&header->e_ident[3]==ELFMAG3)){
		if(header->e_ident[4]!=1){kernel_print("Only32bit supported");return 0;}
		if(header->e_ident[5]!=1){kernel_print("Only little endian supported");return 0;}
		ELFSECTION * sections = (ELFSECTION *)((long)imagelocation + header->e_shoff);
		kernel_print("header sections :: ");
		putL((long)imagelocation + header->e_shoff);
		kernel_print("\n");
		if(header->e_type==1){
			// kernel is for relocation
			// zet alle locaties goed voor de address
			for(unsigned int i = 0 ; i < header->e_shnum ; i++){
				kernel_print("sectionoffset ");
				putL((long)sections[i].sh_offset);
				kernel_print(" initialaddress ");
				putL((long)sections[i].sh_addr);
				kernel_print(" assignedaddress ");
				putL((long)imagelocation + sections[i].sh_offset);
				kernel_print("\n");
				sections[i].sh_addr = (long)imagelocation + sections[i].sh_offset;
			}
			unsigned int i = 0;
			ELFSECTION * shstrtab = NULL;
			for (unsigned int x = 0; x < (unsigned int)header->e_shentsize * header->e_shnum; x += header->e_shentsize) {
				ELFSECTION * shdr = (ELFSECTION *)((unsigned long)header + (header->e_shoff + x));
				if (i == header->e_shstrndx) {
					shstrtab = (ELFSECTION *)((unsigned long)header + shdr->sh_offset);
				}
				i++;
			}
			if(shstrtab==NULL){
				kernel_print("Unable to find stringnamestab\n");return 0;
			}
			// symbolennamenlijstzoeken
			ELFSECTION * symstrtab = NULL;
			for (unsigned int x = 0; x < (unsigned int)header->e_shentsize * header->e_shnum; x += header->e_shentsize) {
				ELFSECTION * shdr = (ELFSECTION *)((unsigned long)header + (header->e_shoff + x));
				if (shdr->sh_type == SHT_STRTAB && (!cmpstr((char *)((unsigned long)shstrtab + shdr->sh_name), ".strtab",7))) {
					symstrtab = (ELFSECTION *)((unsigned long)header + shdr->sh_offset);
				}
			}
			if(symstrtab==NULL){
				kernel_print("Unable to find symbolnames\n");return 0;
			}
			// zoekt sommige vooropgezette onderdelen
			// TODO: vooropgezette onderdelen zoeken
			//
			// symboltable zoeken
			ELFSECTION * sym_shdr = NULL;
			for (unsigned int x = 0; x < (unsigned int)header->e_shentsize * header->e_shnum; x += header->e_shentsize) {
			ELFSECTION * shdr = (ELFSECTION *)((unsigned long)header + (header->e_shoff + x));
			if (shdr->sh_type == SHT_SYMTAB) {
				sym_shdr = shdr;
			}
}
			// Symbolen plaatsen

			for(unsigned int i = 0 ; i < header->e_shnum ; i++){
				ELFSECTION * section = &sections[i];
				if(sections[i].sh_type == SHT_REL){
					ELFRELOCATION * relsection = (void *)section->sh_addr;
					ELFRELOCATION * table = relsection;
					ELFSYMBOL * symtable = (ELFSYMBOL *)(sym_shdr->sh_addr);
					while (((unsigned long)table - (section->sh_addr)) < section->sh_size) {
						ELFSYMBOL * sym = &symtable[ELF32_R_SYM(table->r_info)];
						ELFSECTION * rs = (ELFSECTION *)((unsigned long)header + (header->e_shoff + section->sh_info * header->e_shentsize));
						unsigned long addend = 0;
						unsigned long place  = 0;
						unsigned long symbol = 0;
						unsigned long *ptr = NULL;
						if (ELF32_ST_TYPE(sym->st_info) == STT_SECTION) {
							kernel_print("STT ");
							ELFSECTION * s = (ELFSECTION *)((unsigned long)header + (header->e_shoff + sym->st_shndx * header->e_shentsize));
							ptr = (unsigned long *)(table->r_offset + rs->sh_addr);
							addend = *ptr;
							place  = (unsigned long)ptr;
							symbol = s->sh_addr;
						} else {
							kernel_print("OTH ");
							char * name = (char *)((unsigned long)(header + symstrtab->sh_offset + sym->st_name));
							kernel_print("[");
							kernel_print(name);
							kernel_print("] ");
							ptr = (unsigned long *)(table->r_offset + rs->sh_addr);
							addend = *ptr;
							place  = (unsigned long)ptr;
//							if (!hashmap_get(symboltable, name)) {
//								debug_print(ERROR, "Wat? Missing symbol %s", name);
//							}
							symbol = (unsigned long)&kernel_print;
						}
						switch (ELF32_R_TYPE(table->r_info)) {
							case 1:
								*ptr = addend + symbol;
								kernel_print("Relocationoption 1\n");
								break;
							case 2:
								*ptr = addend + symbol - place;
								kernel_print("Relocationoption 2\n");
								break;
							default:
								kernel_print("Unsupported Relocationoption!\n");
						}
						table++;
					}
					kernel_print("\n");
				}
			}
		}else if(header->e_type==2){
			// kernel is executable
			// Laad alle items in memory
			for(unsigned int i = 0 ; i < header->e_shnum ; i++){
				ELFSECTION section = sections[i];
				if(section.sh_addr){
					for(unsigned int i = 0 ; i < section.sh_size ; i++){
						((char*)section.sh_addr)[i] = ((char*)section.sh_offset)[i];
					}
				}
			}
			return header->e_entry;
		}else{
			// kernel is invalid
			kernel_print("Invalid exec type");
		}
	}else{
		kernel_print("Invalid elfheader!\n");return 0;
	}
	return 0;// 0 = error
}
