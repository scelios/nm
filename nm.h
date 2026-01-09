#ifndef FT_NM_H
# define FT_NM_H

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <endian.h>
#include <math.h>
#include <stdbool.h>


typedef struct s_sym {
	char *name;
	uint64_t addr;
	unsigned char letter;
	unsigned short shndx;
	int used;
}	t_sym;

typedef struct s_nm_flags {
	int a;
	int g;
	int u;
	int r;
	int p;
} t_nm_flags;

int elf64_symbols(Elf64_Sym sym, Elf64_Shdr *shdr, char *file_data, Elf64_Ehdr *elf_header);
int elf32_symbols(Elf32_Sym sym, Elf32_Shdr *shdr, char *file_data, Elf32_Ehdr *elf_header);

int handle64(char *file_data, Elf64_Ehdr *elf_header, struct stat fd_info, t_nm_flags flags);
int handle32(char *file_data, Elf32_Ehdr *elf_header, struct stat fd_info, t_nm_flags flags);

void printTable(t_sym *tab, size_t tab_size, t_nm_flags flags, int bits);
bool str_is_nullterm(const char *start, const char *end);
int	ft_strncmp(const char *first, const char *second, size_t length);
void	ft_putstr_fd(int fd, char *s);
void	print_hex(uint64_t nbr, int width);
size_t	ft_strlen(const char *theString);


void sort(t_sym *syms, int size, t_nm_flags flags);
void mergeSort(t_sym *syms, int l, int r);


uint64_t read_uint64(uint64_t nb, char *file);
uint32_t read_uint32(uint32_t nb, char *file);
uint16_t read_uint16(uint16_t nb, char *file);

int check_endian(char *file);
bool need_change_endian(char *file);
void reverse_endian(void *ptr, size_t length);

#endif