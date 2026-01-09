#include "nm.h"

/*
** format_check64
**
** Description:
**   Validates the ELF64 header and checks for basic file integrity.
**
** Why:
**   We need to ensure the file claims to be a valid ELF64 file and that
**   the section headers are within reasonable bounds before we attempt to parse them.
*/
int format_check64(Elf64_Ehdr *elf_header, struct stat fd_info)
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
	if ((long unsigned int)fd_info.st_size <= sizeof(Elf64_Ehdr))
	{
		ft_putstr_fd(2, "Symbol table or string table not found\n");
		return -1;
	}
	if (elf_header->e_ident[EI_CLASS] != ELFCLASS64 && elf_header->e_ident[EI_DATA] != ELFDATA2LSB\
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
** elf64_symbols
**
** Description:
**   Determines the character code (e.g., 'T', 'U', 'D') for a given symbol
**   based on its type, bind, section index, and flags.
**
** Why:
**   The `nm` command output requires a specific single-character identifier for each symbol
**   to indicate its nature (Text, Data, Undefined, etc.). This function implements that logic
**   according to ELF specifications.
*/
int elf64_symbols(Elf64_Sym sym, Elf64_Shdr *shdr, char *file_data, Elf64_Ehdr *elf_header)
{
	char c = '?';
	uint64_t flags;
	uint64_t bind = ELF64_ST_BIND(sym.st_info);
	uint64_t type = ELF64_ST_TYPE(sym.st_info);
	uint16_t shndx = read_uint16(sym.st_shndx, file_data);
	uint64_t shnum = read_uint16(elf_header->e_shnum, file_data);

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
		type = read_uint64(shdr[shndx].sh_type, file_data);
		flags = read_uint64(shdr[shndx].sh_flags, file_data);

		if (type == SHT_NOBITS)
			c = 'B';
		else if (!(flags & SHF_ALLOC))
			c = 'N';
		else if (!(flags & SHF_WRITE))
		{
			if (flags & SHF_EXECINSTR)
				c = 'T';
			else
				c = 'R';
		}
		else if (flags & SHF_EXECINSTR)
			c = 'T';
		else
			c = 'D';
	}
	if (bind == STB_LOCAL && c != '?')
		c += 32;
	return c;
}

/*
** printTable moved to utils.c
*/

/*
** handle64_symtab
**
** Description:
**   Processes the symbol table section (SHT_SYMTAB). Extracts symbol information,
**   resolves names from the string table, sorts them, and initiates printing.
**
** Why:
**   This is where the raw ELF symbol data is converted into our internal `t_sym` structure list.
**   We need to read offsets, values, and names to build the list for `nm` output.
*/
int handle64_symtab(Elf64_Shdr *section_h, Elf64_Ehdr *elf_header, char *file_data, int n, t_nm_flags flags)
{
	uint64_t sh_offset = read_uint64(section_h[n].sh_offset, file_data);
	uint64_t sh_link = read_uint32(section_h[n].sh_link, file_data);
	Elf64_Sym *symtab = (Elf64_Sym *)(file_data + sh_offset);
	size_t symtab_size = read_uint64(section_h[n].sh_size, file_data) / read_uint64(section_h[n].sh_entsize, file_data);
	char *strtab = file_data + read_uint64(section_h[sh_link].sh_offset, file_data);

	// Get section header string table for symbols with empty names
	uint16_t shstrndx = read_uint16(elf_header->e_shstrndx, file_data);
	char *shstrtab = file_data + read_uint64(section_h[shstrndx].sh_offset, file_data);

	t_sym *tab = malloc(sizeof(t_sym) * symtab_size);
	size_t tab_size = 0;
	if (!tab)
	{
		ft_putstr_fd(2, "Memory allocation failed\n");
		return -1;
	}
	for (size_t i = 1; i < symtab_size; i++)
	{
		uint64_t type = ELF64_ST_TYPE(symtab[i].st_info);
		int keep = 0;
		if (flags.a)
			keep = 1;
		else if (type == STT_FUNC || type == STT_OBJECT || type == STT_NOTYPE || type == STT_GNU_IFUNC || type == STT_TLS)
			keep = 1;
		
		if (keep)
		{
			tab[tab_size].addr = read_uint64(symtab[i].st_value, file_data);
			tab[tab_size].letter = elf64_symbols(symtab[i], section_h, file_data, elf_header);
			tab[tab_size].shndx = read_uint16(symtab[i].st_shndx, file_data);

			uint32_t name_idx = read_uint32(symtab[i].st_name, file_data);
			if (name_idx != 0)
			{
				if (!str_is_nullterm(strtab + name_idx, strtab + read_uint64(section_h[sh_link].sh_size, file_data)))
					tab[tab_size].name = "(null)";
				else
					tab[tab_size].name = strtab + name_idx;
			}
			else if (flags.a)
			{
				// Use section name if symbol name is empty
				uint16_t sec_idx = read_uint16(symtab[i].st_shndx, file_data);
				if (sec_idx < read_uint16(elf_header->e_shnum, file_data))
					tab[tab_size].name = shstrtab + read_uint32(section_h[sec_idx].sh_name, file_data);
				else
					tab[tab_size].name = ""; 
			}
			else
			{
				tab[tab_size].name = "";
			}
			tab_size++;
		}
	}

	sort(tab, tab_size, flags);
	printTable(tab, tab_size, flags, 64);
	
	free(tab);
	return 0;
}

/*
** handle64
**
** Description:
**   Iterates over the ELF64 section headers to find the Symbol Table (SHT_SYMTAB).
**
** Why:
**   We need to find where symbols are stored in the ELF file. Once found,
**   we pass control to `handle64_symtab` to parse them.
*/
int handle64(char *file_data, Elf64_Ehdr *elf_header, struct stat fd_info, t_nm_flags flags)
{
	uint64_t sh_type;
	uint64_t offset = read_uint64(elf_header->e_shoff, file_data);
	uint64_t nEntrie = read_uint16(elf_header->e_shnum, file_data);
	uint16_t index = read_uint16(elf_header->e_shstrndx, file_data);
	Elf64_Shdr *section_h = (Elf64_Shdr *)(file_data + offset);

	if (format_check64(elf_header, fd_info) < 0)
		return -1;
	
	for (uint64_t i = 0; i < nEntrie; i++)
	{
		if (read_uint32(section_h[i].sh_name, file_data) > read_uint64(section_h[index].sh_size, file_data))
		{
			ft_putstr_fd(2, "Bad section table header\n");
			return 1;
		}

		sh_type = read_uint32(section_h[i].sh_type, file_data);
		if (sh_type == SHT_SYMTAB)
			return handle64_symtab(section_h, elf_header, file_data, i, flags);
	}
	ft_putstr_fd(2, "Symbol table or string table not found\n");
	return 1;
}

