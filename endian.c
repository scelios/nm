#include "nm.h"

static int get_machine_endian()
{
	int i;

	i = 1;
	if (*((char*)&i) == 1)
		return (ELFDATA2LSB);
	else
		return (ELFDATA2MSB);
}

bool need_change_endian(char *file)
{
	static unsigned char file_endian;
	static unsigned char mach_endian;
	static int init = 0;

	if (init == 0)
	{
		file_endian = file[EI_DATA];
		mach_endian = get_machine_endian();
		init = 1;
	}
	if (file_endian == mach_endian)
		return (false);
	else
		return (true);
}
uint64_t read_uint64(uint64_t nb, char *file)
{
	bool endian = need_change_endian(file);

	if (endian)
		reverse_endian(&nb, sizeof(uint64_t));
	return (nb);
}

uint32_t read_uint32(uint32_t nb, char *file)
{
	bool endian = need_change_endian(file);

	if (endian)
		reverse_endian(&nb, sizeof(uint32_t));
	return (nb);
}

uint16_t read_uint16(uint16_t nb, char *file)
{
	bool endian = need_change_endian(file);

	if (endian)
		reverse_endian(&nb, sizeof(uint16_t));
	return (nb);
}

void reverse_endian(void *ptr, size_t length)
{
	uint8_t *bytes = (uint8_t *)ptr;
	uint8_t tmp[32];
	size_t i = -1;

	while (++i < length)
	{
		tmp[i] = bytes[length - i - 1];
	}
	i = -1;
	while (++i < length)
	{
		bytes[i] = tmp[i];
	}
}