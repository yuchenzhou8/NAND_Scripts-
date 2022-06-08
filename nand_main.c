#include "nand.h"

/* mtd device */
#define nand_data_dev "/dev/mtd2"
#define 


/* Option definitions */
enum type_option{
    NAND_WRITE,
    NAND_DUMP,
    NAND_ERASE
};

/* Options */
static int8_t type = -1; /* type to execute write/erase/dump */


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
                    else{
                        fprintf(stderr, "ERROR: \"%s\" is not among {write|dump|erase}\n", (char*)optarg);
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
            else{
                fprintf(stderr, "ERROR: \"%s\" is not among {w|d|e}\n", optarg);
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

int main(int argc, char const *argv[])
{
    if(argc != 3)
    {
        printf("Usage: .exe -t {w|d|e}\n");      
        printf("Usage: .exe -type {write|dump|erase}\n");
        exit(EXIT_FAILURE);
    }

    process_options(argc, argv);
    
    if(type == NAND_WRITE)
    {
        nand_write(nand_data_dev,); 
        printf("NAND_WRITE\n");
    }
    else if(type == NAND_DUMP)
    {
        nand_dump(nand_data_dev,);
        printf("NAND_DUMP\n");
    }
    else if(type == NAND_ERASE)
    {
        nand_erase(nand_data_dev,);
        printf("NAND_ERASE\n");
    }
    else
        perror("Invalid NAND Operation\n");
    
    
    return 0;
}
