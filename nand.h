#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <getopt.h>
#include <asm/types.h>
#include "mtd/mtd-user.h"

int nand_erase(const char *device_name, const int offset, const int len);
int nand_write_file(const char *device_name, const char *file_name, const int mtd_offset);
int nand_dump(const char *device_name, void * buffer, int32_t size, const int mtd_offset);
int nand_write(const char *device_name, void * data, int32_t size, const int mtd_offset);