.PHONY: all clean install login

obj-m := debugfs1.o debugfs2.o debugfs3.o

all:
	make -C ../../../output M=$(PWD) modules

clean:
	make -C ../../../linux M=$(PWD) clean

install:
	cp *.ko ../../../elkdat; cd ../../../elkdat; vagrant rsync

login:
	cd ../../../elkdat; vagrant ssh
