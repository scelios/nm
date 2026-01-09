#include "nm.h"

/*
** ft_nm
**
** Description:
**   This is the core function of the nm program. It handles file checking,
**   memory mapping, and dispatching to the appropriate handler (32-bit or 64-bit).
**
** Why:
**   We need to verify if the file is valid, not a directory, and map it into memory
**   for efficient access. Checking the ELF magic numbers allows us to determine
**   if it's a valid ELF file and which architecture to use.
*/
int ft_nm(int fd, char *filename, t_nm_flags flags) {
	struct stat file_info;
	char *data;

	if (fstat(fd, &file_info) < 0 || S_ISDIR(file_info.st_mode))
	{
		ft_putstr_fd(2, "ft_nm: '");
		ft_putstr_fd(2, filename);
		ft_putstr_fd(2, "': Unable to stat file\n");
		return 1;
	}

	data = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (data == MAP_FAILED)
	{
		ft_putstr_fd(2, "ft_nm: '");
		ft_putstr_fd(2, filename);
		ft_putstr_fd(2, "': Unable to map file\n");
		return 1;
	}


	if (data[EI_MAG0] == ELFMAG0 && data[EI_MAG1] == ELFMAG1 && data[EI_MAG2] == ELFMAG2 && data[EI_MAG3] == ELFMAG3)
	{
		if (data[EI_CLASS] == ELFCLASS64)
			return handle64(data, (Elf64_Ehdr *)data, file_info, flags);
		else if (data[EI_CLASS] == ELFCLASS32)
			return handle32(data, (Elf32_Ehdr *)data, file_info, flags);
	}
	else
	{
		ft_putstr_fd(2, "ft_nm: '");
		ft_putstr_fd(2, filename);
		ft_putstr_fd(2, "': File format not recognized\n");
		return 1;
	}

	if (munmap(data, file_info.st_size) < 0)
	{
		ft_putstr_fd(2, "ft_nm: '");
		ft_putstr_fd(2, filename);
		ft_putstr_fd(2, "': Unable to unmap file\n");
		return 1;
	}
	return 0;
}

/*
** parse_flags
**
** Description:
**   Parses command-line arguments to set configuration flags for the nm program.
**   Supported flags: -a (all), -g (extern only), -u (undefined only),
**   -r (reverse sort), -p (no sort).
**
** Why:
**   To support bonus features and standard nm behavior options. We extract flags
**   first so the remaining arguments are treated as filenames.
*/
int parse_flags(int *argc, char ***argv, t_nm_flags *flags)
{
	int i = 1;
	int j;

	flags->a = 0;
	flags->g = 0;
	flags->u = 0;
	flags->r = 0;
	flags->p = 0;

	while (i < *argc)
	{
		if ((*argv)[i][0] == '-')
		{
			j = 1;
			while ((*argv)[i][j])
			{
				if ((*argv)[i][j] == 'a') flags->a = 1;
				else if ((*argv)[i][j] == 'g') flags->g = 1;
				else if ((*argv)[i][j] == 'u') flags->u = 1;
				else if ((*argv)[i][j] == 'r') flags->r = 1;
				else if ((*argv)[i][j] == 'p') flags->p = 1;
				else 
				{
					ft_putstr_fd(2, "ft_nm: invalid option -- '");
					write(2, &(*argv)[i][j], 1);
					ft_putstr_fd(2, "'\n");
					return -1;
				}
				j++;
			}
			// remove this arg
			for (int k = i; k < *argc - 1; k++)
				(*argv)[k] = (*argv)[k + 1];
			(*argc)--;
		}
		else
		{
			i++;
		}
	}
	return 0;
}

/*
** main
**
** Description:
**   Entry point of the program. Parses flags and iterates over input files.
**
** Why:
**   Standard C entry point. It orchestrates the flow: parse flags -> loop files
**   -> call ft_nm for each. Defaulting to "a.out" if no file is provided is standard behavior.
*/
int main(int argc, char **argv) {
	int fd;
	int ret;
	t_nm_flags flags;

	if (parse_flags(&argc, &argv, &flags) < 0)
		return 1;

	if (argc == 1)
	{
		fd = open("a.out", O_RDONLY);
		if (fd == -1)
		{
			ft_putstr_fd(2, "ft_nm: 'a.out': No such file\n");
			exit(1);
		}
		ret = ft_nm(fd, "a.out", flags);
		close(fd);
	}
	else
	{
		// if multiple files, print filename before output if not just one
		int print_fname = (argc > 2);
		
		for (int i = 1; i < argc; i++)
		{
			fd = open(argv[i], O_RDONLY);
			if (fd == -1)
			{
				ft_putstr_fd(2, "ft_nm: '");
				ft_putstr_fd(2, argv[i]);
				ft_putstr_fd(2, "': No such file\n");
			}
			else
			{
				if (print_fname)
				{
					ft_putstr_fd(1, "\n");
					ft_putstr_fd(1, argv[i]);
					ft_putstr_fd(1, ":\n");
				}
				ret = ft_nm(fd, argv[i], flags);
				close(fd);
			}
		}
	}

	ret == 0 ? exit(0) : exit(1);
}