.PHONY: all clean install login

obj-m := hello.o

all:
	make -C ../../../output M=$(PWD) modules

clean:
	make -C ../../../linux M=$(PWD) clean

install:
	cp *.ko ../../../elkdat; cd ../../../elkdat; vagrant rsync

login:
	cd ../../../elkdat; vagrant ssh
