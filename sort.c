#include "nm.h"

int	cmp_sym(t_sym a, t_sym b, t_nm_flags flags)
{
	int ret = ft_strncmp(a.name, b.name, 100000); // effectively strcmp
	if (ret != 0)
		return (flags.r ? -ret : ret);
	
	if (a.addr < b.addr)
		return (flags.r ? 1 : -1);
	if (a.addr > b.addr)
		return (flags.r ? -1 : 1);
	return (0);
}

void sort(t_sym *array, int size, t_nm_flags flags)
{
	int i;
	int j;
	t_sym temp;

	if (flags.p)
		return;

	i = 0;
	while (i < size)
	{
		j = i + 1;
		while (j < size)
		{
			if (cmp_sym(array[i], array[j], flags) > 0)
			{
				temp = array[i];
				array[i] = array[j];
				array[j] = temp;
			}
			j++;
		}
		i++;
	}
}
