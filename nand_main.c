#include "nand.h"

/* mtd device */
#define NAND_DATA_DEV       "/dev/mtd2"
#define NAND_FLASH_OFFSET   0

/* Option definitions */
enum type_option{
    NAND_WRITE,
    NAND_DUMP,
    NAND_ERASE,
    NAND_UPDATE
};

static void process_options(int argc, char const *argv[]);
static uint16_t compute_crc(const void *DataPtr, uint16_t DataLength, uint16_t InputCRC);

/* Options */
static int8_t type = -1; /* type to execute write/erase/dump */

/* Example GENSAT-1 Info to Save */
typedef struct _GENSAT_1_cFS_preserved_data_
{
    int16_t     deploy_state;               /* Deployment Status of GENSAT-1, just a boolean variable */
    int16_t     num_launch_state;           /* Number of cFS launched */
    uint16_t    crc_check;                  /* CRC Check */
    int16_t     spare;                      /* 4-bytes Alignment */
}GENSAT_1_cFS_preserved_data;


/* Test data */
GENSAT_1_cFS_preserved_data test = {0, 0, 0, 0};
/* Pointer to test structure */
GENSAT_1_cFS_preserved_data * ptest = &test;


int main(int argc, char const *argv[])
{
    if(argc != 3)
    {
        printf("Usage: .exe -t {w|d|e|u}\n");      
        printf("Usage: .exe -type {write|dump|erase|update}\n");
        exit(EXIT_FAILURE);
    }

    process_options(argc, argv);
    int32_t status  = 0;
    
    if(type == NAND_WRITE)
    {
        if(ptest->deploy_state == 0)
        {
            ptest->deploy_state++;
        }
        ptest->num_launch_state++;
        ptest->crc_check = compute_crc((const void *)ptest, sizeof(int16_t)*2, 0);
        ptest->spare = 0;

        status = nand_write(NAND_DATA_DEV, (const void *)ptest, sizeof(GENSAT_1_cFS_preserved_data), NAND_FLASH_OFFSET); 
        printf("NAND_WRITE\n");
    }
    else if(type == NAND_DUMP)
    {
        status = nand_dump(NAND_DATA_DEV, ptest, sizeof(GENSAT_1_cFS_preserved_data), NAND_FLASH_OFFSET);

        if(status == 0)
        {
            uint16_t crc_local = compute_crc((const void *)ptest, sizeof(int16_t)*2, 0);
            if(crc_local != ptest->crc_check)
            {
                printf("miss match crc!\n");
            }
            printf("deploy_state %d, num_launch_state %d\n", ptest->deploy_state, ptest->num_launch_state);
        }
        printf("NAND_DUMP\n");
    }
    else if(type == NAND_ERASE)
    {
        status = nand_erase(NAND_DATA_DEV, NAND_FLASH_OFFSET, sizeof(GENSAT_1_cFS_preserved_data));
        printf("NAND_ERASE\n");
    }
    /* update the test structure */
    else if(type == NAND_UPDATE)
    {
        /* First step: Dump and verify the NAND data */
        status = nand_dump(NAND_DATA_DEV, ptest, sizeof(GENSAT_1_cFS_preserved_data), NAND_FLASH_OFFSET);

        if(status == 0)
        {

            /* Check if the NAND Flash is used*/

            if(ptest->deploy_state == 0xFF || ptest->num_launch_state == 0xFF || ptest->crc_check == 0xFF || ptest->spare == 0xFF)
            {
                printf("First Launch of cFS, initialize NAND data!\n");
                ptest->deploy_state = 0;
                ptest->num_launch_state = 1;
                ptest->crc_check = compute_crc((const void *)ptest, sizeof(int16_t)*2, 0);
                ptest->spare = 0;
                printf("deploy_state %d, num_launch_state %d\n", ptest->deploy_state, ptest->num_launch_state);
            }
            else
            {
                uint16_t crc_local = compute_crc((const void *)ptest, sizeof(int16_t)*2, 0);

                if(crc_local != ptest->crc_check)
                {
                    printf("Miss Match CRC!\n");
                    exit(EXIT_FAILURE);
                }

                /* Update NAND Flash */
                if(ptest->deploy_state == 0)
                {
                    ptest->deploy_state = 1;
                }
                ptest->num_launch_state++;
                ptest->crc_check = compute_crc((const void *)ptest, sizeof(int16_t)*2, 0);
                ptest->spare = 0;

                printf("deploy_state %d, num_launch_state %d\n", ptest->deploy_state, ptest->num_launch_state);
            }

            /* Erase the NAND flash block so we can write to it*/
            status = nand_erase(NAND_DATA_DEV, NAND_FLASH_OFFSET, sizeof(GENSAT_1_cFS_preserved_data));

            if(status == 0)
            {
                status = nand_write(NAND_DATA_DEV, (const void *)ptest, sizeof(GENSAT_1_cFS_preserved_data), NAND_FLASH_OFFSET); 

                if(status == 0)
                {
                    printf("NAND_UPDATE: NAND_WRITE success\n");
                }
                else
                {
                    printf("NAND_UPDATE: NAND_WRITE failed\n");
                }
            }
            else
            {
                printf("NAND_UPDATE: NAND_ERASE failed to Erase\n");
            }

        }
        else
        {
            printf("NAND_UPDATE: failed to dump NAND data, exit!\n");
            exit(EXIT_FAILURE);
        }


    }
    else
        perror("Invalid NAND Operation\n");
    
    
    return 0;
}


static uint16_t compute_crc(void *DataPtr, uint16_t DataLength, uint16_t InputCRC)
{
    uint32_t  i;
    int16_t  Index;
    int16_t Crc = 0;
    const uint8_t *BufPtr;
    uint8_t  ByteValue;

    static const uint16_t CrcTable[256]=
    {

		    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040

    };

    Crc    =  (int16_t )( 0xFFFF & InputCRC );
    BufPtr = (const uint8_t *)DataPtr;

    for ( i = 0 ; i < DataLength ; i++,  BufPtr++)
    {
        /*
        * It is assumed that the supplied buffer is in a
        * directly-accessible memory space that does not
        * require special logic to access
        */
        ByteValue = *BufPtr;
        Index = ( ( Crc ^ ByteValue) & 0x00FF);
        Crc = ( (Crc >> 8 ) & 0x00FF) ^ CrcTable[Index];
    }

    return Crc;
}

static void process_options(int argc, char const *argv[])
{
    int32_t error  = 0; 
    int32_t c = 0;
    bool run = true;

    while(run)
    {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"t", required_argument, 0, 't'},
            {"type", required_argument, 0, 0}
        };

        c = getopt_long(argc, argv, "t:", long_options, &option_index);

        if(c == -1) break;

        switch (c)
        {
        case 0:
            if(!strcmp("type", long_options[option_index].name))
            {
                if(optarg)
                {
                    if(!strcmp("write",optarg))
                        type = NAND_WRITE;
                    else if(!strcmp("dump",optarg))
                        type = NAND_DUMP;
                    else if(!strcmp("erase",optarg))
                        type = NAND_ERASE;
                    else if(!strcmp("update",optarg))
                        type = NAND_UPDATE;
                    else{
                        fprintf(stderr, "ERROR: \"%s\" is not among {write|dump|erase|update}\n", (char*)optarg);
                        run = false;
                    }
                }
                else
                {
                    perror("ERROR: -type does not have an option argument.\n");
                    run = false;
                }
            }
            break;

        case 't':
            if(!strcmp("w",optarg))
                type = NAND_WRITE;
            else if(!strcmp("d",optarg))
                type = NAND_DUMP;
            else if(!strcmp("e",optarg))
                type = NAND_ERASE;
            else if(!strcmp("u",optarg))
                type = NAND_UPDATE;
            else{
                fprintf(stderr, "ERROR: \"%s\" is not among {w|d|e|u}\n", optarg);
                run = false;
            }
            break;
        
        default:
            printf("?? getopt returned character code 0%o ??\n", c);
            break;
        }
    }
    
    if(!run)
        exit(EXIT_FAILURE);
    
    printf("type = %d\n", type);
    
}