.PHONY: all clean

obj-m := timer1.o timer2.o timer3.o timer4.o

all:
	make -C ../../../output M=$(PWD) modules

clean:
	make -C ../../../linux M=$(PWD) clean
