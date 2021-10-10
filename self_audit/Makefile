obj-m:= my_audit.o
CURRENT_PATH:=$(shell pwd)
LINUX_KERNEL_PATH:=/app/soource/linux-5.5

all:
        make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
        make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean