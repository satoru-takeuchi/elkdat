.PHONY: all clean

obj-m := hello.o

all:
	make -C ../../../output M=$(PWD) modules

clean:
	make -C ../../../linux M=$(PWD) clean
