#include "nand.h"

int nand_erase(const char *device_name, const int offset, const int len) {

    int fd;
    int ret = 0;
    struct stat st;
    mtd_info_t meminfo;
    erase_info_t erase;
 
    //open mtd device
    fd = open(device_name, O_RDWR);
    if (fd < 0) {
        printf("open %s failed!\n", device_name);
        return -1;
    }
 
    //check is a char device
    ret = fstat(fd, &st);
    if (ret < 0) {
        printf("fstat %s failed!\n", device_name);
        close(fd);
        return -1;
    }
 
    if (!S_ISCHR(st.st_mode)) {
        printf("%s: not a char device", device_name);
        close(fd);
        return -1;
    }
 
    //get meminfo
    ret = ioctl(fd, MEMGETINFO, &meminfo);
    if (ret < 0) {
        printf("get MEMGETINFO failed!\n");
        close(fd);
        return -1;
    }
 
    erase.length = meminfo.erasesize;
 
    for (erase.start = offset; erase.start < offset + len; erase.start += meminfo.erasesize) {
        loff_t bpos = erase.start;
 
        //check bad block
        ret = ioctl(fd, MEMGETBADBLOCK, &bpos);
        if (ret > 0) {
            printf("mtd: not erasing bad block at 0x%08llx\n", bpos);
            continue;  // Don't try to erase known factory-bad blocks.
        }
 
        if (ret < 0) {
            printf("MEMGETBADBLOCK error");
            close(fd);
            return -1;
        }
 
        //erase
        if (ioctl(fd, MEMERASE, &erase) < 0) {
            printf("mtd: erase failure at 0x%08llx\n", bpos);
            close(fd);
            return -1;
        }
    }
 
    close(fd);
    return 0;
}


static unsigned next_good_eraseblock(int fd, struct mtd_info_user *meminfo, unsigned block_offset)
{
    while (1) {
        loff_t offs;
 
        if (block_offset >= meminfo->size) {
            printf("not enough space in MTD device");
            return block_offset; /* let the caller exit */
        }
 
        offs = block_offset;
        if (ioctl(fd, MEMGETBADBLOCK, &offs) == 0)
            return block_offset;
 
        /* ioctl returned 1 => "bad block" */
        printf("Skipping bad block at 0x%08x\n", block_offset);
        block_offset += meminfo->erasesize;
    }
}
 
 
int nand_write_file(const char *device_name, const char *file_name, const int mtd_offset) {
 
    mtd_info_t meminfo;
    unsigned int blockstart;
    unsigned int limit = 0;
    int cnt = -1;
    int size = 0;
    int ret = 0;
    int offset = mtd_offset;
 
    //fopen input file
    FILE *pf = fopen(file_name, "r");
    if (pf==NULL) {
        printf("fopen %s failed!\n", file_name);
        return -1;
    }
 
    //open mtd device
    int fd = open(device_name, O_WRONLY);
    if (fd < 0) {
        printf("open %s failed!\n", device_name);
        fclose(pf);
        return -1;
    }
 
    //get meminfo
    ret = ioctl(fd, MEMGETINFO, &meminfo);
    if (ret < 0) {
        printf("get MEMGETINFO failed!\n");
        fclose(pf);
        close(fd);
        return -1;
    }
 
    limit = meminfo.size;
 
    //check offset page aligned
    if (offset & (meminfo.writesize - 1)) {
        printf("start address is not page aligned");
        fclose(pf);
        close(fd);
        return -1;
    }
 
    //malloc buffer for read 
    char *tmp = (char *)malloc(meminfo.writesize);
    if (tmp == NULL) {
        printf("malloc %d size buffer failed!\n", meminfo.writesize);
        fclose(pf);
        close(fd);
        return -1;
    }
 
    //if offset in a bad block, get next good block
    blockstart = offset & ~(meminfo.erasesize - 1);
    if (offset != blockstart) {
        unsigned int tmp;
        tmp = next_good_eraseblock(fd, &meminfo, blockstart);
        if (tmp != blockstart) {
            offset = tmp;
        }
    }
 
    while(offset < limit) {
        blockstart = offset & ~(meminfo.erasesize - 1);
        if (blockstart == offset) {
            offset = next_good_eraseblock(fd, &meminfo, blockstart);
            printf("Writing at 0x%08x\n", offset);
 
            if (offset >= limit) {
                printf("offset(%d) over limit(%d)\n", offset, limit);
                fclose(pf);
                close(fd);
                free(tmp);
                return -1;
            }
        }
 
        lseek(fd, offset, SEEK_SET);
 
        cnt = fread(tmp, 1, meminfo.writesize, pf);
        if (cnt == 0) {
            printf("write ok!\n");
            break;
        }
 
        if (cnt < meminfo.writesize) {
            /* zero pad to end of write block */
            memset(tmp + cnt, 0, meminfo.writesize - cnt);
        }
 
        size = write(fd, tmp, meminfo.writesize);
        if (size != meminfo.writesize) {
            printf("write err, need :%d, real :%d\n", meminfo.writesize, size );
            fclose(pf);
            close(fd);
            free(tmp);
            return -1;
        }
 
        offset += meminfo.writesize;
 
        if (cnt < meminfo.writesize) {
            printf("write ok!\n");
            break;
        }
    }
 
    //free buf
    free(tmp);
    fclose(pf);
    close(fd);
 
    return 0;//test
 
}

int nand_write(const char *device_name, void * data, int32_t size, const int mtd_offset) {
 
    mtd_info_t meminfo;
    unsigned int blockstart;
    unsigned int limit = 0;
    int cnt = -1;
    int size = 0;
    int ret = 0;
    int offset = mtd_offset;
 
    //fopen input file
    FILE *pf = fopen(file_name, "r");
    if (pf==NULL) {
        printf("fopen %s failed!\n", file_name);
        return -1;
    }
 
    //open mtd device
    int fd = open(device_name, O_WRONLY);
    if (fd < 0) {
        printf("open %s failed!\n", device_name);
        fclose(pf);
        return -1;
    }
 
    //get meminfo
    ret = ioctl(fd, MEMGETINFO, &meminfo);
    if (ret < 0) {
        printf("get MEMGETINFO failed!\n");
        fclose(pf);
        close(fd);
        return -1;
    }
 
    limit = meminfo.size;
 
    //check offset page aligned
    if (offset & (meminfo.writesize - 1)) {
        printf("start address is not page aligned");
        fclose(pf);
        close(fd);
        return -1;
    }
 
    //malloc buffer for read 
    char *tmp = (char *)malloc(meminfo.writesize);
    if (tmp == NULL) {
        printf("malloc %d size buffer failed!\n", meminfo.writesize);
        fclose(pf);
        close(fd);
        return -1;
    }
 
    //if offset in a bad block, get next good block
    blockstart = offset & ~(meminfo.erasesize - 1);
    if (offset != blockstart) {
        unsigned int tmp;
        tmp = next_good_eraseblock(fd, &meminfo, blockstart);
        if (tmp != blockstart) {
            offset = tmp;
        }
    }
 
    while(offset < limit) {
        blockstart = offset & ~(meminfo.erasesize - 1);
        if (blockstart == offset) {
            offset = next_good_eraseblock(fd, &meminfo, blockstart);
            printf("Writing at 0x%08x\n", offset);
 
            if (offset >= limit) {
                printf("offset(%d) over limit(%d)\n", offset, limit);
                fclose(pf);
                close(fd);
                free(tmp);
                return -1;
            }
        }
 
        lseek(fd, offset, SEEK_SET);
 
        cnt = fread(tmp, 1, meminfo.writesize, pf);
        if (cnt == 0) {
            printf("write ok!\n");
            break;
        }
 
        if (cnt < meminfo.writesize) {
            /* zero pad to end of write block */
            memset(tmp + cnt, 0, meminfo.writesize - cnt);
        }
 
        size = write(fd, tmp, meminfo.writesize);
        if (size != meminfo.writesize) {
            printf("write err, need :%d, real :%d\n", meminfo.writesize, size );
            fclose(pf);
            close(fd);
            free(tmp);
            return -1;
        }
 
        offset += meminfo.writesize;
 
        if (cnt < meminfo.writesize) {
            printf("write ok!\n");
            break;
        }
    }
 
    //free buf
    free(tmp);
    fclose(pf);
    close(fd);
 
    return 0;//test
 
}


int nand_dump(const char *device_name, void * data, int32_t size, const int mtd_offset){
    return 0;
}