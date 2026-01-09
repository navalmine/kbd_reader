obj-m += kbd_sim.o

KDIR ?= /lib/modules/$(shell uname -r)/build
LLVM ?= 1
CC ?= clang

all:
	$(MAKE) -C $(KDIR) M=$(PWD) LLVM=$(LLVM) CC=$(CC) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	
#bear -- make -j generate compile_commands.json