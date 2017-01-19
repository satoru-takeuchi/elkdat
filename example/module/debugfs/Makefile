.PHONY: all clean

obj-m := debugfs1.o debugfs2.o debugfs3.o

all:
	make -C ../../../output M=$(PWD) modules

clean:
	make -C ../../../linux M=$(PWD) clean
