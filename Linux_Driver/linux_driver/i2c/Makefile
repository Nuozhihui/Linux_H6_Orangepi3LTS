
KERNELDIR := /home/kemp/work/rk3399/rk3399_embeded/kernel
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
obj-m := gy906.o

