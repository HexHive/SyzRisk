# SyzRisk Fuzzer

Adapted [Syzkaller](https://github.com/google/syzkaller) for SyzRisk.

## Usage

Specify `weight_map` in the Syzkaller configuration to set the weight profile
from the matcher engine (the JSON file).  Everything else is _exactly_
the same. The example configuration:

```diff
{
  "target": "linux/amd64",
  "http": "127.0.0.1:56741",
  "workdir": "workdir",
  "kernel_obj": "linux-6.0",
  "image": "image/stretch.img",
  "sshkey": "image/stretch.id_rsa",
  "syzkaller": "syzkaller",
  "procs": 6,
  "type": "qemu",
+ "weight_map": "func_weights.json",
  "vm": {
    "count": 4,
    "kernel": "linux-6.0/arch/x86/boot/bzImage",
    "mem": 2048
  }
}
```

## Note

 - It's based on the commit `2b253ced7f2f` (Sep 21, 2022) and selectively
   applied the "Bluetooth HCI" patch from `6feb842be06b` (Nov 7, 2022) as the
   corresponding Syzkaller bug caused a too severe slowdown.

 - As long as the JSON file follows the same format as the engine-generated one, it'll work just as fine. You can fiddle with different weighting strategies using this (if you need to).

 - The change is so minimal that it should be able to extract it with one
   `.patch` file, but there is no point in doing this if it is applicable to
   only _one specific version_ of Syzkaller; why not just upload the
   `.patch`-applied one? In this
   regard, I would highly appreciate it if somebody could make this into a
   _version-generic_ patch that can be applied to most of the recent Syzkaller
   versions. It should be doable with scripts.
