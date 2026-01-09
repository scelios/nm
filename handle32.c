#include "nm.h"

/*
** format_check32
**
** Description:
**   Validates the ELF32 header and checks for basic file integrity.
**
** Why:
**   Ensures the file claims to be a valid ELF32 file and that
**   section headers are accessible. Prevents processing invalid files.
*/
int format_check32(Elf32_Ehdr *elf_header, struct stat fd_info)
{
	if (!elf_header)
	{
		ft_putstr_fd(2, "Invalid ELF header\n");
		return -1;
	}

	// check machine version
	if (elf_header->e_machine == EM_NONE)
	{
		ft_putstr_fd(2, "Architecture not handled\n");
		return -1;
	}

	// check header max size
	if ((long unsigned int)fd_info.st_size <= sizeof(Elf32_Ehdr))
	{
		ft_putstr_fd(2, "Symbol table or string table not found\n");
		return -1;
	}
	if (elf_header->e_ident[EI_CLASS] != ELFCLASS32 && elf_header->e_ident[EI_DATA] != ELFDATA2LSB\
		&& elf_header->e_type != ET_EXEC && elf_header->e_type != ET_DYN)
	{	
		ft_putstr_fd(2, "Symbol table or string table not found\n");
		return -1;
	}
	
	// check if e_shnum is within bounds
	if (elf_header->e_shnum >= SHN_LORESERVE)
	{	
		ft_putstr_fd(2, "Too many sections\n");
		return -1;
	}
	return 0;
}

/*
** elf32_symbols
**
** Description:
**   Determines the character code (e.g., 'T', 'U', 'D') for a given symbol
**   based on its type, bind, section index, and flags.
**
** Why:
**   Specific to 32-bit ELF structures. Maps the internal ELF symbol type
**   to the standard single-letter code used by `nm`.
*/
int elf32_symbols(Elf32_Sym sym, Elf32_Shdr *shdr, char *file_data, Elf32_Ehdr *elf_header)
{
	char c = '?';
	uint32_t flags;
	uint32_t bind = ELF32_ST_BIND(sym.st_info);
	uint32_t type = ELF32_ST_TYPE(sym.st_info);
	uint16_t shndx = read_uint16(sym.st_shndx, file_data);
	uint32_t shnum = read_uint16(elf_header->e_shnum, file_data);

	if (bind == STB_GNU_UNIQUE)
		c = 'u';
	else if(type == STT_GNU_IFUNC)
		c = 'i';
	else if (bind == STB_WEAK)
	{
		if(type == STT_OBJECT)
			c = (shndx == SHN_UNDEF) ? 'v' : 'V';
		else
			c = (shndx == SHN_UNDEF) ? 'w' : 'W';
	}
	else if (bind == STB_WEAK)
	{
		c = 'W';
		if (shndx == SHN_UNDEF)
			c = 'w';
	}
	else if (sym.st_shndx == SHN_UNDEF)
		c = 'U';
	else if (sym.st_shndx == SHN_ABS)
		c = 'A';
	else if (sym.st_shndx == SHN_COMMON)
		c = 'C';
	else if (shndx < shnum)
	{
		type = read_uint32(shdr[shndx].sh_type, file_data);
		flags = read_uint32(shdr[shndx].sh_flags, file_data);

		if (type == SHT_NOBITS)
			c = 'B';
		else if (!(flags & SHF_WRITE))
		{
		if(flags & SHF_ALLOC && flags & SHF_EXECINSTR)
			c = 'T';
		else
			c = 'R';
		}
		else if(flags & SHF_EXECINSTR)
			c = 'T';
		else
			c = 'D';
	}
	if (bind == STB_LOCAL && c != '?')
		c += 32;
	return c;
}

/*
** handle32_symtab
**
** Description:
**   Processes the symbol table section (SHT_SYMTAB) for 32-bit ELF files.
**   Extracts, parses, sorts, and prints the symbols.
**
** Why:
**   Reads the symbol table entries, resolves strings from the string table,
**   converts them to our internal `t_sym` format, and handles the display logic.
*/
int handle32_symtab(Elf32_Shdr *section_h, Elf32_Ehdr *elf_header, char *file_data, int n, t_nm_flags flags)
{
	uint32_t sh_offset = read_uint32(section_h[n].sh_offset, file_data);
	uint32_t sh_link = read_uint32(section_h[n].sh_link, file_data);
	Elf32_Sym *symtab = (Elf32_Sym *)(file_data + sh_offset);
	size_t symtab_size = read_uint32(section_h[n].sh_size, file_data) / read_uint32(section_h[n].sh_entsize, file_data);
	char *strtab = file_data + read_uint32(section_h[sh_link].sh_offset, file_data);
	t_sym *tab = malloc(sizeof(t_sym) * symtab_size);
	size_t tab_size = 0;
	if (!tab)
	{
		ft_putstr_fd(2, "Memory allocation failed\n");
		return -1;
	}
	for (size_t i = 1; i < symtab_size; i++)
	{
		uint32_t type = ELF32_ST_TYPE(symtab[i].st_info);
		int keep = 0;
		if (flags.a)
			keep = 1;
		else if (type == STT_FUNC || type == STT_OBJECT || type == STT_NOTYPE || type == STT_GNU_IFUNC || type == STT_TLS)
			keep = 1;
		
		if (keep)
		{
			tab[tab_size].addr = read_uint32(symtab[i].st_value, file_data);
			tab[tab_size].letter = elf32_symbols(symtab[i], section_h, file_data, elf_header);
			tab[tab_size].shndx = read_uint16(symtab[i].st_shndx, file_data);
			if (!str_is_nullterm(strtab + read_uint32(symtab[i].st_name, file_data), strtab + read_uint32(section_h[sh_link].sh_size, file_data)))
				tab[tab_size].name = "(null)";
			else
				tab[tab_size].name = strtab + read_uint32(symtab[i].st_name, file_data);
			tab_size++;
		}
	}

	sort(tab, tab_size, flags);
	printTable(tab, tab_size, flags, 32);
	
	free(tab);
	return 0;
}

/*
** handle32
**
** Description:
**   Iterates over the ELF32 section headers to find the Symbol Table (SHT_SYMTAB).
**
** Why:
**   The entry point for 32-bit ELF processing. Finds the symbol table and delegates
**   to `handle32_symtab`.
*/
int handle32(char *file_data, Elf32_Ehdr *elf_header, struct stat fd_info, t_nm_flags flags)
{
	uint32_t sh_type;
	uint32_t offset = read_uint32(elf_header->e_shoff, file_data);
	uint32_t nEntrie = read_uint16(elf_header->e_shnum, file_data);
	uint16_t index = read_uint16(elf_header->e_shstrndx, file_data);
	Elf32_Shdr *section_h = (Elf32_Shdr *)(file_data + offset);

	if (format_check32(elf_header, fd_info) < 0)
		return -1;
	
	for (uint32_t i = 0; i < nEntrie; i++)
	{
		if (read_uint32(section_h[i].sh_name, file_data) > read_uint32(section_h[index].sh_size, file_data))
		{
			ft_putstr_fd(2, "Bad section table header\n");
			return 1;
		}

		sh_type = read_uint32(section_h[i].sh_type, file_data);
		if (sh_type == SHT_SYMTAB)
			return handle32_symtab(section_h, elf_header, file_data, i, flags);
	}
	ft_putstr_fd(2, "Symbol table or string table not found\n");
	return 1;
}

