#################################################
#
#
# Pre pre pre pre pre pre pre pre alpha
#
#
################################################## 
CC = gcc

ifeq (,$(BUILD_KERNEL))
BUILD_KERNEL=$(shell uname -r)
endif


KDIR = /lib/modules/$(BUILD_KERNEL)/build
PWD = $(shell pwd)
INSTDIR := $(DESTDIR)/lib/modules/$(BUILD_KERNEL)/kernel/net/bf
TARGETDIR := $(DESTDIR)/usr/src/$(BUILD_KERNEL)/include/linux/

#DEST = /lib/modules/$(BUILD_KERNEL)/misc
#EXTRA_CFLAGS += -O3 -std=gnu99 --no-warnings
CMD_SRC = bf_cmd.c 
CMD_OBJ = $(patsubst %.c,obj/%.o,$(CMD_SRC)) 

TARGET = bf_filter
OBJS = bf_module.o nl_int.o 

obj-m := $(TARGET).o 
$(TARGET)-objs := $(OBJS)

all: default cmd

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

$(TARGET).o: $(OBJS)
	$(LD) -r -o $@ $(OBJS)


cmd:    $(CMD_SRC)
	$(CC) $(CMD_SRC) -o bf_cmd  -lnl  -g


clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order
	@rm -f .*.*.cmd *.symvers *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions
	
install:
	mkdir -p $(INSTDIR)
	cp *.ko $(INSTDIR)	
	cp trx_data.h $(DESTDIR)/usr/include/linux
	@if test -d ${TARGETDIR}; then \
		cp trx_data.h ${TARGETDIR}; \
	fi
ifeq (,$(DESTDIR))
	/sbin/depmod $(BUILD_KERNEL)
endif
