# elkdat

easy linux kernel development and auto-test tool

elkdat automatically setups linux kernel source repository and a VM for
linux kernel development and test. In addition, it can build, install,
boot, run your own tests, test the all patches in a patchset, and find
a problematic commit with bisect by just one command.

It's useful not only for experienced linux kernel developers, but also for
kernel newbies and non-developers who'd like to test linux kernel.

# Prerequisite environment

You need to have a Linux system with virtualization feature. It the following command shows one or more lines, yoor machine has that feature.

```
$ egrep '^flags.*(vmx|svm)' /proc/cpuinfo
```

You should install some packages before initializing elkdat. Here is how to install them on Ubuntu 18.04.

```
$ sudo apt-get install git vagrant libvirt-bin libvirt-dev kernel-package qemu-kvm libssl-dev libncurses5-dev
$ sudo usermod -aG libvirtd <your user name>
```

If you use other distros, please install corresponding packages.

# How to initialize

Just run `./init` underneath the top of this tool.

Please note that elkdat downloads whole linux kernel source repository in `./init`.
If you already have it, please copy it underneath the top directory as "linux".
In addition, elkdat flushes existing kernel, object files and so on by `mrproper`.

# How to finalize

Run the following command.

```
$ ./fini
...
Finished to cleanup.
Now it's safe to delete the source directory.
$ 
```

# Managing test VM

You can boot test VM as follows.

```
$ ./up
Bringing machine 'ktest' up with 'libvirt' provider...
==> ktest: Starting domain.
==> ktest: Waiting for domain to get an IP address...
==> ktest: Waiting for SSH to become available...
==> ktest: Creating shared folders metadata...
==> ktest: Rsyncing folder: /home/sat/src/elkdat/elkdat/ => /vagrant
==> ktest: Machine already provisioned. Run `vagrant provision` or use the `--provision`
==> ktest: flag to force provisioning. Provisioners marked to run always will still run.
$ 
```

Please note that test VM is already up just after running init command.


After that, you can login to test VM as follows.

```
$ ./login 
Welcome to Ubuntu 18.04.1 LTS (GNU/Linux 4.18.0-ktest x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/advantage

Last login: Mon Sep 24 16:32:47 2018 from 192.168.121.1
vagrant@localhost:~$ 
```

You can shutdown test VM as follows.

```
$ ./halt
==> ktest: Halting domain...
$ 
```

You can also reboot test VM as follows.


```
$ ./reload
==> ktest: Halting domain...
==> ktest: Starting domain.
==> ktest: Waiting for domain to get an IP address...
==> ktest: Waiting for SSH to become available...
==> ktest: Creating shared folders metadata...
==> ktest: Rsyncing folder: /home/sat/src/elkdat/elkdat/ => /vagrant
==> ktest: Machine already provisioned. Run `vagrant provision` or use the `--provision`
==> ktest: flag to force provisioning. Provisioners marked to run always will still run.
$ 

```

# Tutorial

From here, we assume the current directory is underneath the top directory.

## Run your own kernel (don't change source)

Let's boot linux v5.0. You can build, install, and boot linux v5.0
just by the following several commands.

```
$ cd linux
$ git checkout v5.0
$ cd ../
$ ./test boot
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
$ ./login
...
vagrant@localhost:~$ uname -r
4.18.0-ktest
vagrant@localhost:~$ 
```

Finally we restarts the previous (probably the distro's) kernel.

```
vagrant@localhost:~$ exit
$ ./reload
$ 
```

You can use your own kernel again by the following commands.

```
$ ./login
...
vagrant@localhost:~$ sudo su 
root@localhost:/home/vagrant# grub-reboot ktest
root@localhost:/home/vagrant# exit
exit 
vagrant@localhost:~$ exit
...
$ ./reload
...
$ 
```

You can boot the any kernel version by changing the above mentioned _v5.0_ in
`git checkout` command to any tag or to any commit ID.

If it's OK to just build or just install your own kernel rather than booting it,
Please use `./test build` or `./test install` instead of `./test boot`.

## Run your own kernel (change source)

Here we apply a trivial patch, printing a simple message to kernel log, to linux v4.18 and boot it.

Let's take a look at the patch.

```
$ cat example/kernel-patch/first/0001-Print-a-message-on-boot.patch 
commit 20c7774d8c3b055b9cfa330d457b5a3baf5f01bf (HEAD -> sat)
Author: Satoru Takeuchi <satoru.takeuchi@gmail.com>
Date:   Tue Sep 25 01:52:18 2018 +0900

    test

diff --git a/init/main.c b/init/main.c
index 5e13c544bbf4..e4b57366ddac 100644
--- a/init/main.c
+++ b/init/main.c
@@ -590,6 +590,7 @@ asmlinkage __visible void __init start_kernel(void)
 	mm_init();
 
 	ftrace_init();
+	printk("my patch is applied!\n");
 
 	/* trace_printk can be enabled here */
 	early_trace_init();
```

Make a kernel and boot it.

```
$ cd linux
$ git checkout -b test v4.18
Switched to a new branch 'test'
$ git am ../example/kernel-patch/first/0001-Print-a-message-on-boot.patch 
Applying: test
$ cd ../
$ ./test boot
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
$ ./login
...
vagrant@localhost:~$ uname -r
4.18.0-ktest+
vagrant@localhost:~$ 
```

Read the kernel log.

```
vagrant@localhost:~$ dmesg | grep "my patch"
[    0.000000] my patch is applied!
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

static int mymodule_init(void) {
        printk(KERN_ALERT "Hello world!\n");
        return 0;
}

static void mymodule_exit(void) {
        /* Do nothing */
}

module_init(mymodule_init);
module_exit(mymodule_exit);
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
$ make install
...
$ 
```

Load it. Please not that it only succeed if the test VM is booted with
your own kernel.

```
$ make login
...
vagrant@localhost:~$ sudo su
root@localhost:/home/vagrant# insmod /vagrant/hello.ko
root@localhost:/home/vagrant# 
```

See the kernel log.

```
root@localhost:/home/vagrant# dmesg | tail -3
[  314.198886] random: crng init done
[  516.935519] hello: loading out-of-tree module taints kernel.
[  516.936950] Hello world!
root@localhost:/home/vagrant# 
```

Succeeded!

## Change the kernel configuration

You can change the kernel configuration, for example for enable a file system disabled by default,
by executing the following commands before running `./test`.

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

### Run your own tests

Run the following command.
```
$ ./test test <the path of your own test>
```

For example, the following command runs the example/test/hello after booting the system.

```
$ ./test test example/test/hello
...
** Monitor flushed **
run test /home/sat/src/elkdat/example/test/hello
/home/sat/src/elkdat/example/test/hello ... [0 seconds] SUCCESS
kill child process 18446
closing!

Build time:   6 minutes 53 seconds
Install time: 8 seconds
Reboot time:  17 seconds
Test time:    1 second



*******************************************
*******************************************
KTEST RESULT: TEST 1 SUCCESS!!!!         **
*******************************************
*******************************************

    1 of 1 tests were successful

$ 
```

example/test/hello's output is in ktest/ktest.log.

Here is the result of running example/test/false, it always fails.

```
$ ./test test example/test/fail
...
** Monitor flushed **
run test /home/sat/src/elkdat/example/test/fail
/home/sat/src/elkdat/example/test/fail ... [0 seconds] FAILED!
CRITICAL FAILURE... test failed
REBOOTING
ssh -i /home/sat/src/elkdat/private_key root@192.168.121.181 sync ... [1 second] SUCCESS
ssh -i /home/sat/src/elkdat/private_key root@192.168.121.181 reboot; ... Connection to 192.168.121.181 closed by remote host.
[0 seconds] SUCCESS
 See /home/sat/src/elkdat/ktest/ktest.log for more info.
test failed
$ 
```

### Test your patchset one by one

If you're a kernel developer, you make and submit a patchset for each feature
or bug fix. Of course you should test it before submitting it. Then testing the kernel,
which applies whole patchset, is insufficient. You should test individual patches instead.
It's because if a patch in your patchset has a BUG, it would corrupt git-bisect after
applying your patchset. elkdat can test whole patchset one by one automatically.

Here is the [example](https://github.com/satoru-takeuchi/elkdat/tree/master/example/kernel-patch/patchcheck)
of a patchset consists of four patches and its 3rd one causes panic during boot.
These patches are quite simple. Please take a look at each patches if you're interested in it.

```
$ git am ../example/kernel-patch/patchcheck/*
$ git log --oneline -5
706e8dec17ba (HEAD -> patchcheck, checkpatch) 4/4 fine
dc95a48e92b6 3/4 BUG
0141c43b9cf1 2/4 fine
0602bde62d19 1/4 fine
94710cac0ef4 (tag: v4.18) Linux 4.18
$ 
```

To test this patchset, run the follownig command.

```
$ cd ../
./test patchcheck 0602bde62d19 706e8dec17ba boot
...
Going to test the following commits:
0602bde62d19ec5268a8411de70e9c59d39aff5d 1/4 fine
0141c43b9cf1b3a022ae058c23fb81b2a7ddeae6 2/4 fine
dc95a48e92b6b02d2dea2ca6a2ed6326e8f02c55 3/4 BUG
706e8dec17bac0cc9bac4d5ac7c39d75994d1e7e 4/4 fine

Processing commit "0602bde62d19ec5268a8411de70e9c59d39aff5d 1/4 fine"
...

Build time:   3 minutes 13 seconds
Install time: 8 seconds
Reboot time:  24 seconds

Processing commit "0141c43b9cf1b3a022ae058c23fb81b2a7ddeae6 2/4 fine"
...
Build time:   31 seconds
Install time: 7 seconds
Reboot time:  23 seconds

Processing commit "dc95a48e92b6b02d2dea2ca6a2ed6326e8f02c55 3/4 BUG"
...
[5 seconds] FAILED!
power cycle
cd ../elkdat; vagrant halt; sleep 5; vagrant up ... [77 seconds] SUCCESS
** Wait for monitor to settle down **

** Monitor flushed **
 See /home/sat/src/elkdat/ktest/ktest.log for more info.
failed - never got a boot prompt.
~/src/elkdat/ktest ~/src/elkdat
~/src/elkdat
$ 
```

Fails on testing 3/4 patch. We succeeded.

### Find which commit introduce a bug by bisect

If you found a kernel didn't work and you also know which kernel worked fine,
`./test bisect` can be used to detect the wrong commit. It works as `git bisect`[^1].
It's difficult to use `git bisect` directly in kernel development since it requires to
reboot whole system on test one commit.

[^1]: Please refer to `man 1 git-bisect` if you don't know about this command.

Here is the [example](https://github.com/satoru-takeuchi/elkdat/tree/master/example/kernel-patch/bisect)
of a patchset consists of ten patches and its 6th one causes panic during boot.
These patches are quite simple. Please take a look at each patches if you're interested in it.

```
$ git am ../example/kernel-patch/bisect/*
$ git log --oneline -11
2ff43f7d52d0 (HEAD -> bisect) 10/10 BUG
0c841b3a35cd 9/10 BUG
001f75285253 8/10 BUG
bae066f6e395 7/10 BUG
56b54e93122b 6/10 BUG
2d3dbf6afe6b 5/10 fine
6b42ac68fa8a 4/10 fine
5694f923f6f7 3/10 fine
468bbf83f8a6 2/10 fine
6d4155d1aecf 1/10 fine
94710cac0ef4 (tag: v4.18) Linux 4.18
$ 
```

To find bad commit which introduce a bug, so it's 6/10 patch, run the following command.

```
$ ../
$ ./test bisect 6d4155d1aecf 2ff43f7d52d0 boot
...
RUNNING TEST 1 of 1 with option bisect boot

git rev-list --max-count=1 6d4155d1aecf ... SUCCESS
git rev-list --max-count=1 2ff43f7d52d0 ... SUCCESS
git bisect start ... [1 second] SUCCESS
git bisect good 6d4155d1aecf51fd878d00143ed2d176e976e587 ... [0 seconds] SUCCESS
git bisect bad 2ff43f7d52d0ef95ebfaf0b4ae805689c6a55cd7 ... SUCCESS
Bisecting: 4 revisions left to test after this (roughly 2 steps) [2d3dbf6afe6b7c3ba89f740ec6c9a719ef9cfb50]
...
Bisecting: 2 revisions left to test after this (roughly 1 step) [bae066f6e395a8a884864534ff4a3759ede3814b]

Build time:   3 minutes 13 seconds
Install time: 7 seconds
Reboot time:  24 seconds
...
Bisecting: 0 revisions left to test after this (roughly 0 steps) [56b54e93122b1d5410ce533a85c1ee53c85279aa]

Build time:   3 minutes 14 seconds
Install time: 7 seconds
Reboot time:  2 minutes 7 seconds
...
git bisect bad ... SUCCESS
Found bad commit... 56b54e93122b1d5410ce533a85c1ee53c85279aa

Build time:   3 minutes 14 seconds
Install time: 13 seconds
Reboot time:  2 minutes 8 seconds
git bisect log ... [0 seconds] SUCCESS
git bisect reset ... [0 seconds] SUCCESS
Bad commit was [56b54e93122b1d5410ce533a85c1ee53c85279aa]



*******************************************
*******************************************
KTEST RESULT: TEST 1 SUCCESS!!!!         **
*******************************************
*******************************************

    1 of 1 tests were successful

~/src/elkdat/ktest ~/src/elkdat
~/src/elkdat
$ 
```

We successfully detected a correct bad commit, 6/10.

### Others 

./test works as a wrapper of _ktest_, a kernel auto test tool. 
Please refer to [linux kernel auto test by using ktest](http://www.slideshare.net/satorutakeuchi18/kernel-auto-testbyktest)
for more information about ktest.
