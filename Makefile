CC=gcc
ARM_CC=arm-linux-gnueabi-gcc
CFLAG=-w


all: nand nand_arm

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