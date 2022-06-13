CC=gcc
ARM_CC=arm-linux-gnueabi-gcc
CFLAG=-w


all: nand nand_arm

nand_data_update: nand_update nand_update_arm

nand_update: nand_update.o
	$(CC) nand_update.o -o nand_update

nand_update_arm: nand_update_arm.o
	$(ARM_CC) nand_update_arm.o -o nand_update_arm

nand_update.o: nand_data_update.c
	$(CC) -c nand_data_update.c -o nand_update.o

nand_update_arm.o: nand_data_update.c
	$(ARM_CC) -c nand_data_update.c -o nand_update_arm.o

nand_arm: nand_main_arm.o nand_arm.o
	$(ARM_CC) nand_main_arm.o nand_arm.o -o nand_arm

nand: nand_main.o nand.o
	$(CC) nand_main.o nand.o -o nand

nand_main_arm.o: nand_main.c
	$(ARM_CC) -c nand_main.c -o nand_main_arm.o

nand_arm.o: nand.c
	$(ARM_CC) -c nand.c -o nand_arm.o

nand_main.o:nand_main.c
	$(CC) -c nand_main.c

nand.o: nand.c
	$(CC) -c nand.c

clean: 
	rm -rvf *.o nand nand_arm