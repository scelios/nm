#include "nm.h"

void	print_hex(uint64_t nbr, int width)
{
	char	*base = "0123456789abcdef";
	char	buffer[17]; 
	int		i;

	i = 0;
	if (nbr == 0)
		buffer[i++] = '0';
	else
	{
		while (nbr > 0)
		{
			buffer[i++] = base[nbr % 16];
			nbr /= 16;
		}
	}
    // Pad with zeros
    while (i < width)
        buffer[i++] = '0';
    
    // Print in reverse
	while (--i >= 0)
		write(1, &buffer[i], 1);
}
