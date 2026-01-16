#include "common.h"

int	get_file(const char *filename, int *fd)
{
	*fd = open(filename, O_RDONLY);

	return (*fd < 0);
}

int get_file_size(int fd, off_t *file_size)
{
	*file_size = lseek(fd, 0, SEEK_END);
	if (*file_size == -1)
	{
		perror("Failed to seek the end of file");
		return (-2);
	}
	if (lseek(fd, 0, SEEK_SET) == -1)
	{
		perror("Failed to seek the begin of file");
		return (-2);
	}
	return (0);
}
