.PHONY: all clean

obj-m := list1.o

all:
	make -C ../../../output M=$(PWD) modules

clean:
	make -C ../../../linux M=$(PWD) clean
