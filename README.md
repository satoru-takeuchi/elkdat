# elkdat

easy linux kernel development and auto-test tool

elkdat automatically setups linux kernel source repository and a VM for
linux kernel development and test. In addition, it can build, install,
boot, run your own tests, test the all patches in a patchset, and find
a problematic commit with bisect by just one command.

It's useful not only for experienced linux kernel developers, but also for
kernel newbies and non-developers who'd like to test linux kernel.

# Required packages

You should install some packages before installing elkdat. Here is how to install them on Ubuntu 16.04.

```
# apt-get install git vagrant libvirt-bin libvirt-dev kernel-package qemu-kvm libssl-dev libncurses5-dev
```

In addition to install the above mentioned package, you need to install vagrant-libvirt plugin for vagrant as follows.

```
$ sudo sed -i'' "s/Specification.all = nil/Specification.reset/" /usr/lib/ruby/vendor_ruby/vagrant/bundler.rb         # See https://github.com/vagrant-libvirt/vagrant-libvirt/issues/575 for more details about this patching
$ vagrant plugin install vagrant-libvirt
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

# Tutorial

From here, we assume the current directory is underneath the top directory.

## Run your own kernel (don't change source)

Let's boot linux v4.9. You can build, install, and boot linux v4.9
just by the following several commands.

```
$ cd linux
$ git checkout v4.9
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
$ cd elkdat
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

If it's OK to just build or just install your own kernel rather than booting it,
Please use `./test build` or `./test install` instead of `./test boot`.

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
        /* Do nothing */
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
$ make install
...
$ 
```

Load it. Please not that it only succeed if the test VM is booted with
your own kernel.

```
$ make login
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
$ git log --oneline -5 
f80a34f377c1 4/4: fine again
227ef171c7f5 3/4: BUG
d662eff22070 2/4: fine
925417fc1d36 1/4: fine
69973b830859 Linux 4.9
$ 
```

To test this patchset, run the follownig command.

```
$ ./test patchcheck 925417fc1d36 f80a34f377c1 boot
...
Going to test the following commits:
925417fc1d3670f994c26bb09369b5f6c02c60bb 1/4: fine
d662eff220707c43c7bce87cf0343e27e67ce848 2/4: fine
227ef171c7f59c570fb821a81581ef78eed5be89 3/4: BUG
f80a34f377c1832d450dc0cc402288ee86ae2836 4/4: fine again

Processing commit "925417fc1d3670f994c26bb09369b5f6c02c60bb 1/4: fine"
...
Build time:   6 minutes 58 seconds
Install time: 8 seconds
Reboot time:  21 seconds

Processing commit "d662eff220707c43c7bce87cf0343e27e67ce848 2/4: fine"
...
** Monitor flushed **
kill child process 30367
closing!

Build time:   1 minute 15 seconds
Install time: 9 seconds
Reboot time:  19 seconds

Processing commit "227ef171c7f59c570fb821a81581ef78eed5be89 3/4: BUG"

[    0.135879] ftrace: allocating 32412 entries in 127 pages
[    0.163806] 1/4 patch is applied!
[    0.164408] 2/4 patch is applied!
[    0.164933] 3/4 patch is applied!
[    0.165469] ------------[ cut here ]------------
[    0.166151] kernel BUG at /home/sat/src/elkdat/linux/init/main.c:663!
[    0.167041] invalid opcode: 0000 [#1] SMP
[    0.167647] Modules linked in:
[    0.168216] CPU: 0 PID: 0 Comm: swapper/0 Not tainted 4.9.0-ktest+ #3
[    0.169076] Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS 1.9.3-20161025_171302-gandalf 04/01/2014
[    0.170451] task: ffffffff8ee0e540 task.stack: ffffffff8ee00000
[    0.171254] RIP: 0010:[<ffffffff8ef81fd9>]  [<ffffffff8ef81fd9>] start_kernel+0x460/0x462
[    0.172501] RSP: 0000:ffffffff8ee03f50  EFLAGS: 00010282
[    0.173241] RAX: 0000000000000015 RBX: ffffffffffffffff RCX: ffffffff8ee54108
[    0.174175] RDX: 0000000000000000 RSI: 0000000000000246 RDI: 0000000000000246
[    0.175109] RBP: ffffffff8ee03f80 R08: 0000000000000000 R09: 0000000000000000
[    0.176043] R10: ffff9b179ffd7000 R11: 0000000000000098 R12: ffff9b179ffd06c0
[    0.176987] R13: ffffffff8f030840 R14: ffffffff8f03d2e0 R15: 000000000008a000
[    0.177924] FS:  0000000000000000(0000) GS:ffff9b179fc00000(0000) knlGS:0000000000000000
[    0.179070] CS:  0010 DS: 0000 ES: 0000 CR0: 0000000080050033
[    0.179853] CR2: 00000000ffffffff CR3: 0000000013e07000 CR4: 00000000000406f0
[    0.180831] Stack:
[    0.181216]  ffffffff8f03d2e0 0000000000000000 000000000000008e 0000ffffffff8ef8
[    0.182537]  0000000000000020 ffffffff8ef81120 ffffffff8ee03f90 ffffffff8ef812ca
[    0.183854]  ffffffff8ee03fe8 ffffffff8ef81419 00000000ffffffff 8ef88e0000101117
[    0.185206] Call Trace:
[    0.185639]  [<ffffffff8ef81120>] ? early_idt_handler_array+0x120/0x120
[    0.186521]  [<ffffffff8ef812ca>] x86_64_start_reservations+0x24/0x26
[    0.187383]  [<ffffffff8ef81419>] x86_64_start_kernel+0x14d/0x170
[    0.188228] Code: 02 00 e8 b7 b1 02 00 48 c7 c7 5d 13 c5 8e e8 e6 a2 21 ff 48 c7 c7 76 13 c5 8e e8 da a2 21 ff 48 c7 c7 8f 13 c5 8e e8 ce a2 21 ff <0f> 0b 31 c0 80 3f 00 55 48 89 e5 75 0f c7 05 4c 80 17 00 01 00 
[    0.194669] RIP  [<ffffffff8ef81fd9>] start_kernel+0x460/0x462
[    0.195523]  RSP <ffffffff8ee03f50>
[    0.196097] ---[ end trace f68728a0d3053b52 ]---
[    0.196776] Kernel panic - not syncing: Attempted to kill the idle task!
[    0.197707] ---[ end Kernel panic - not syncing: Attempted to kill the idle task!
bug timed out after 1 seconds
Test forced to stop after 60 seconds after failure
CRITICAL FAILURE... failed - got a bug report
REBOOTING
ssh -i /home/sat/src/elkdat/private_key root@192.168.121.181 sync ... [18 seconds] FAILED!
ssh -i /home/sat/src/elkdat/private_key root@192.168.121.181 reboot; ... ssh: connect to host 192.168.121.181 port 22: No route to host
[3 seconds] SUCCESS
 See /home/sat/src/elkdat/ktest/ktest.log for more info.
failed - got a bug report
$ 
```

Fails on testing 3/4 patch. We succeeded.


### Find which commit introduce a bug by bysect

If you found a kernel didn't work and you also know which kernel worked fine,
`./test bisect` can be used to detect the wrong commit. It works as `git bisect`[^1].
It's difficult to use `git bisect` directly in kernel development since it requires to
reboot whole system on test one commit.

[^1]: Please refer to `man 1 git-bisect` if you don't know about this command.

Here is the [example](https://github.com/satoru-takeuchi/elkdat/tree/master/example/kernel-patch/bisect)
of a patchset consists of ten patches and its 6th one causes panic during boot.
These patches are quite simple. Please take a look at each patches if you're interested in it.

```
$ git log --oneline -11
e617cb9e8cc0 10/10: BUG
d5159dda90f5 9/10: BUG
ddd7cdeacf47 8/10: BUG
9f6c5fbcd327 7/10: BUG
966f935e572c 6/10: BUG
f4504cce28bc 5/10: fine
cacbea15ec6a 4/10: fine
ee916bd4a2a8 3/10: fine
b61a82b33071 2/10: fine
5b762eff2275 1/10: fine
69973b830859 Linux 4.9
```

To find bad commit which introduce a bug, so it's 6/10 patch, run the following command.

```
$ ./test bisect 5b762eff2275 e617cb9e8cc0 boot
...
RUNNING TEST 1 of 1 with option bisect boot

git rev-list --max-count=1 5b762eff2275 ... SUCCESS
git rev-list --max-count=1 e617cb9e8cc0 ... SUCCESS
git bisect start ... [0 seconds] SUCCESS
git bisect good 5b762eff2275a414938275c00ccae7d2847f10b4 ... [0 seconds] SUCCESS
git bisect bad e617cb9e8cc0b49a507bc2fd2840fb803da00436 ... SUCCESS
Bisecting: 4 revisions left to test after this (roughly 2 steps) [f4504cce28bcb56b15df0c936e1598cb733f1658]
...
git bisect good ... SUCCESS
Bisecting: 2 revisions left to test after this (roughly 1 step) [9f6c5fbcd3276216291f60f41504bab6003c95e6]
...
git bisect bad ... SUCCESS
Bisecting: 0 revisions left to test after this (roughly 0 steps) [966f935e572c728f17877ab8a8fac454e04deda6]
...
...
git bisect bad ... SUCCESS
Found bad commit... 966f935e572c728f17877ab8a8fac454e04deda6
...
Bad commit was [966f935e572c728f17877ab8a8fac454e04deda6]



*******************************************
*******************************************
KTEST RESULT: TEST 1 SUCCESS!!!!         **
*******************************************
*******************************************

    1 of 1 tests were successful

$ 
```

We successfully detected a correct bad commit, 6/10.

### Others 

./test works as a wrapper of _ktest_, a kernel auto test tool. 
Please refer to [linux kernel auto test by using ktest](http://www.slideshare.net/satorutakeuchi18/kernel-auto-testbyktest)
for more information about ktest.
