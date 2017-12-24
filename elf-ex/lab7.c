#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <elf.h>

/* Given the in-memory ELF header pointer as `ehdr` and a section
   header pointer as `shdr`, returns a pointer to the memory that
   contains the in-memory content of the section */
#define AT_SEC(ehdr, shdr) ((void *)(ehdr) + (shdr)->sh_offset)
static void check_for_shared_object(Elf64_Ehdr *ehdr);
static void fail(char *reason, int err_code);
static Elf64_Shdr *section_by_name(Elf64_Ehdr *ehdr,char *section_name);
Elf64_Shdr *section_by_index(Elf64_Ehdr *ehdr, Elf64_Section s);
void print_symbols(Elf64_Ehdr* ehdr);

int main(int argc, char **argv) 
{
  int fd;
  size_t len;
  void *p;
  Elf64_Ehdr *ehdr;

  if (argc != 2)
    fail("expected one file on the command line", 0);

  /* Open the shared-library file */
  fd = open(argv[1], O_RDONLY);
  if (fd == -1)
    fail("could not open file", errno);

  /* Find out how big the file is: */
  len = lseek(fd, 0, SEEK_END);

  /* Map the whole file into memory: */
  p = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (p == (void*)-1)
    fail("mmap failed", errno);

  /* Since the ELF file starts with an ELF header, the in-memory image
     can be cast to a `Elf64_Ehdr *` to inspect it: */
  ehdr = (Elf64_Ehdr *)p;

  /* Check that we have the right kind of file: */
  check_for_shared_object(ehdr);
  
  /* Your work for parts 1-6 goes here */
  Elf64_Shdr *shdrs = (void*)ehdr+ehdr->e_shoff;
  //char *strs = (void*)ehdr+shdrs;
  printf("%d\n",ehdr->e_shoff); // section table
  printf("%d\n",ehdr->e_shstrndx); // index of a section in section table that contains
  									// section-name strings
  char *strs = AT_SEC(ehdr, shdrs +ehdr->e_shstrndx );
  //printf("%s\n",strs + 1);
  /*
  int i ;
  for (i = 0; i < ehdr->e_shnum; i++){
    printf("%d %s\n",i, strs + shdrs[i].sh_name);
  }
  */
  //printf("%d\n", section_by_name(ehdr,".dynsym"));
  Elf64_Shdr *dynsym_shdr = section_by_name(ehdr, ".dynsym");
  Elf64_Sym *syms = AT_SEC(ehdr, dynsym_shdr);
  strs = AT_SEC(ehdr, section_by_name(ehdr, ".dynstr"));

  /*
  int i, count = dynsym_shdr->sh_size / sizeof(Elf64_Sym);
  for (i = 0; i < count; i++) {
  	if (strcmp(strs + syms[i].st_name, "f0") == 0) {
	 	printf("%p\n", syms[i].st_value);
	 	Elf64_Shdr *shdr = section_by_index(ehdr, syms[i].st_shndx);
	 	printf("%p\n",  shdr->sh_addr);
	 	
	 	printf("%x\n",  shdr->sh_addr + (syms[i].st_value - shdr->sh_addr));
  	}
   }

  */
  
  int j = syms[7].st_shndx;
  Elf64_Shdr *shdr = section_by_index(ehdr,syms[7].st_shndx);
  printf("%x\n",shdr->sh_addr);
  unsigned char *ab = AT_SEC(ehdr,shdrs + j);
  // printf("%x %x %x\n",syms[7].st_shndx.sh_addr,syms[7].st_value-shdrs[j].sh_addr, ab[syms[7].st_value - shdrs[j].sh_addr]);
  
  return 0;
}

/* Just do a little bit of error-checking:
   Make sure we're dealing with an ELF file. */
static void check_for_shared_object(Elf64_Ehdr *ehdr) 
{
  if ((ehdr->e_ident[EI_MAG0] != ELFMAG0)
      || (ehdr->e_ident[EI_MAG1] != ELFMAG1)
      || (ehdr->e_ident[EI_MAG2] != ELFMAG2)
      || (ehdr->e_ident[EI_MAG3] != ELFMAG3))
    fail("not an ELF file", 0);

  if (ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    fail("not a 64-bit ELF file", 0);

  if (ehdr->e_type != ET_DYN)
    fail("not a shared-object file", 0);
}

static void fail(char *reason, int err_code) 
{
  fprintf(stderr, "%s (%d)\n", reason, err_code);
  exit(1);
}
static Elf64_Shdr *section_by_name(Elf64_Ehdr *ehdr,char *section_name) {
	Elf64_Shdr *shdrs = (void*)ehdr+ehdr->e_shoff;
	char *strs = AT_SEC(ehdr, shdrs +ehdr->e_shstrndx );
	int i ;
  	for (i = 0; i < ehdr->e_shnum; i++){
    	if (strcmp(strs + shdrs[i].sh_name,section_name) == 0) {
    		return shdrs + i;
    	}
  	}
  	//return -1;
}
Elf64_Shdr *section_by_index(Elf64_Ehdr *ehdr, Elf64_Section s) {
  Elf64_Shdr *shdrs = (void*)ehdr+ehdr->e_shoff;
  //char *strs = AT_SEC(ehdr, shdrs +ehdr->e_shstrndx );
  return shdrs + s;
}
