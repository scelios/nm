#include "nm.h"

int	ft_strncmp(const char *first, const char *second, size_t length)
{
	size_t	i;

	i = 0;
	while (i < length)
	{
		if (((unsigned char)first[i] != (unsigned char)second[i]
				|| first[i] == 0) || second[i] == 0)
			return ((unsigned char)first[i] - (unsigned char)second[i]);
		i++;
	}
	return (0);
}

void	ft_putstr_fd(int fd, char *s)
{
	int	i;

	if (!s)
		return ;
	i = 0;
	while (s[i])
	{
		write(fd, &s[i], 1);
		i++;
	}
}

size_t	ft_strlen(const char *theString)
{
	int	i;

	i = 0;
	while (theString[i])
		i++;
	return (i);
}

bool str_is_nullterm(const char *start, const char *end)
{
	while (start < end)
	{
		if (!*start)
			return true;
		++start;
	}

	return false;
}

/*
** printTable
**
** Description:
**   Iterates through the sorted symbol table and prints each entry that passes
**   the filter flags.
**
** Why:
**   We need to display the results to stdout. This function handles formatting
**   addresses (padding), the type character, and the symbol name. It also respects filters like -u or -g.
*/
void printTable(t_sym *tab, size_t tab_size, t_nm_flags flags, int bits)
{
	for (size_t i = 0; i < tab_size; i++)
	{
		if (flags.u && tab[i].shndx != SHN_UNDEF)
			continue;
		
		if (flags.g && (tab[i].letter >= 'a' && tab[i].letter <= 'z') 
			&& tab[i].letter != 'w' && tab[i].letter != 'v')
			continue;

		if (tab[i].shndx == SHN_UNDEF)
		{
			if (bits == 32)
				ft_putstr_fd(1, "        ");
			else
				ft_putstr_fd(1, "                ");
			ft_putstr_fd(1, " ");
			write(1, &tab[i].letter, 1);
			ft_putstr_fd(1, " ");
			ft_putstr_fd(1, tab[i].name);
			ft_putstr_fd(1, "\n");
		}
		else
		{
			print_hex(tab[i].addr, (bits == 32) ? 8 : 16);
			ft_putstr_fd(1, " ");
			write(1, &tab[i].letter, 1);
			ft_putstr_fd(1, " ");
			ft_putstr_fd(1, tab[i].name);
			ft_putstr_fd(1, "\n");
		}
	}
}
