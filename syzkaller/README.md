# SyzRisk Fuzzer

Adapted [Syzkaller](https://github.com/google/syzkaller) for SyzRisk.

## Usage

Specify `weight_map` in the configuration file to set the weight profile
produced by the matcher engine (the JSON file).  Everything else is _exactly_
the same as in the original Syzkaller. The example configuration file is as
follows.

```
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
  "reproduce": false,
  "weight_map": "weights/pattern.json",   # <<< this right here.
  "vm": {
    "count": 4,
    "kernel": "linux-6.0/arch/x86/boot/bzImage",
    "mem": 2048
  }
}
```

## Note

 - It's based on the commit `2b253ced7f2f` (Sep 21, 2022) and selectively
   applied the "bluetooth hci" patch from `6feb842be06b` (Nov 7, 2022) as the
   corresponding Syzkaller bug caused too severe slowdown.
