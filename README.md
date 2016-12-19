# elkdat
easy linux kernel development and auto-test tool

elkdat prepares linux kernel source and a VM, and setting these properly
for linux kernel development/auto-testing.

It's useful not only for experienced linux kernel developers, but also for
kernel newbies and non-developers who'd like to test linux kernel.

# Required packages

Here is the required packages in debian.

- git
- vagrant
- kernel-package
- vagrant-libvirt
- qemu-kvm
- libvirt-daemon
- libvirt-clients

If you use other distros, please install corresponding packages.

# How to install

Just run `./init` underneath the top of this tool! 

Please note that elkdat downloads whole linux kernel source repository in `./init`.
If you already have it, please copy it underneath the top directory as "linux".
In addition, elkdat flushes existing kernel, object files and so on by `mrproper`.

# Tutorial

From here, we assume the current directory is underneath the top directory.

## Run your own kernel (don't change source)

Let's boot linux v4.9. You can build, install, and boot linux v4.9
just by the following several commands.

```
$ cd linux
$ git checkout v4.9
$ cd ../ktest
$ ./ktest.pl
...
*******************************************
*******************************************
KTEST RESULT: TEST 1 SUCCESS!!!!         **
*******************************************
*******************************************

    1 of 1 tests were successful

$ 
```

Let's login to test VM to confirm whether it works correctly.

```
$ cd ../elkdat
$ vagrant ssh
...
vagrant@packer-qemu:~$ uname -r
4.9.0-ktest
vagrant@packer-qemu:~$ 
```

Finally we restarts the previous (probably the distro's) kernel.

```
vagrant@packer-qemu:~$ exit
$ vagrant halt
...
$ vagrant reload
...
$ cd ../
$ 
```

You can use your own kernel again by the following commands.

```
$ cd elkdat
$ vagrant ssh 
...
vagrant@packer-qemu:~$ sudo su 
root@packer-qemu:/home/vagrant# grub-reboot ktest
root@packer-qemu:/home/vagrant# exit
exit 
vagrant@packer-qemu:~$ exit
...
$ vagrant reload 
...
$ 
```

You can boot the any kernel version by changing the above mentioned _v4.9_ in
`git checkout` command to any tag or to any commit ID.

## Run your own kernel (change source)

Here we apply a trivial patch, printing a simple message to kernel log, to linux v4.9 and boot it.

Let's take a look at the patch.

```
$ cat example/kernel-patch/first/0001-Print-a-message-on-boot.patch 
From 93cc6bf35ed2850634cb1bcfe621b38d81c6ab25 Mon Sep 17 00:00:00 2001
From: Satoru Takeuchi <satoru.takeuchi@gmail.com>
Date: Wed, 14 Dec 2016 20:42:17 +0900
Subject: [PATCH] Print a message on boot

Signed-off-by: Satoru Takeuchi <satoru.takeuchi@gmail.com>
---
 init/main.c | 1 +
 1 file changed, 1 insertion(+)

diff --git a/init/main.c b/init/main.c
index 2858be7..9736dac 100644
--- a/init/main.c
+++ b/init/main.c
@@ -657,6 +657,7 @@ asmlinkage __visible void __init start_kernel(void)
        }

        ftrace_init();
+       printk("my patch is applied!\n");

        /* Do the rest non-__init'ed, we're now alive */
        rest_init();
--
2.10.2

```

Make a kernel and boot it.

```
$ cd linux
$ git checkout -b test v4.9
Switched to a new branch 'test'
$ git am ../example/kernel-patch/first/0001-Print-a-message-on-boot.patch 
Applying: Print a message on boot
$ cd ../ktest
$ ./ktest.pl
...
*******************************************
*******************************************
KTEST RESULT: TEST 1 SUCCESS!!!!         **
*******************************************
*******************************************

    1 of 1 tests were successful

```

Login to confirm whether we succeeded or not.

```
$ cd ../elkdat
$ vagrant ssh
...
vagrant@packer-qemu:~$ uname -r
4.9.0-ktest+
vagrant@packer-qemu:~$ 
```

Read the kernel log.

```
vagrant@packer-qemu:~$ dmesg | grep "my patch"
[    0.167288] my patch is applied!
$ 
```

Succeeded!

Please restart your system here with the previous kernel.

## Load your own kernel module

It's easy to build, install, and load your own kernel module. Here we use
the simple module which just prints "Hello world!" on loading.

Let's take a look at its source.

```
$ cat example/module/hello/hello.c
#include <linux/module.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Satoru Takeuchi <satoru.takeuchi@gmail.com>");
MODULE_DESCRIPTION("Hello world kernel module");

static int hello_init(void) {
        printk(KERN_ALERT "Hello world!\n");
        return 0;
}

static void hello_exit(void) {
        printk(KERN_ALERT "driver unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);
```

Build this module.

```
$ cd example/module/hello
$ make
...
$ 
```

Copy it to the test VM.

```
$ cp hello.ko ../../../elkdat
$ cd ../../../elkdat
$ vagrant rsync
...
$ 
```

Load it. Please not that it only succeed if the test VM is booted with
your own kernel.

```
$ vagrant ssh
...
vagrant@packer-qemu:~$ sudo su
root@packer-qemu:/home/vagrant# insmod /vagrant/hello.ko
root@packer-qemu:/home/vagrant# 
```

See the kernel log.

```
root@packer-qemu:/home/vagrant# dmesg | tail -3
[  314.198886] random: crng init done
[  516.935519] hello: loading out-of-tree module taints kernel.
[  516.936950] Hello world!
root@packer-qemu:/home/vagrant# 
```

Succeeded!

## Change the kernel configuration

You can change the kernel configuration, for example for enable a file system disabled by default,
by executing the following commands before running `./ktest.pl`.

```
$ cp ktest/minconfig{,.bak}
$ cp ktest/minconfig linux/.config
$ cd linux
$ make menuconfig
... 　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　# Here change the configuration as you like
$ mv .config ../ktest/minconfig
$ make mrproper
$ cd ../
```

If the new kernel built from the new configuration doesn't boot,
please restore the configuration file by `ktest/minconfig{.bak,}`.

## Rich tests

In addition to build, install, and boot your own kernel, elkdat has the following features.

- Run your own tests
- Test your patchset one by one
- Find which commit introduce a bug by bysect

All of them are fully automated.

In fact, these features are not elkdat's own feature, _ktest_'s feature.
elkdat uses ktest as backend.
Please refer to [linux kernel auto test by using ktest](http://www.slideshare.net/satorutakeuchi18/kernel-auto-testbyktest)
to how to setup ktest.
