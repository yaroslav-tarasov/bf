#################################################
#
#
# Pre pre pre pre pre pre pre pre alpha
#
#
################################################## 
CC = gcc
CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/misc
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
