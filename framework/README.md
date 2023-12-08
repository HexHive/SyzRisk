# SyzRisk Pre-fuzzing Components

The pre-fuzzing components of SyzRisk, including:

 1. Ground-truth dataset collection (for Linux)
 2. Pattern development
 3. Risk estimation
 4. Weight generation

The recommended usage roughly follows the same order, too.

## Tl;dr

 0. Install.
```
$ ./install.sh
$ . SOURCE_ME
```

 1. Set the path to your (Git-managed) target project.
```
$ kfc-repo <dir/path/to/proj>
```


 3. (optional) Develop your own pattern.

---

## System Requirement

 - OS: Ubuntu 20.04
 - CPU: Intel(R) i7-8665
 - Memory: 16GB

Other OSes _may_ work well without problems, especially on another Linux distro. Smaller memory space can be problematic with some parts (e.g., pattern matching during risk estimation and weight generation).


## Install

 1. Run `install.sh`: `$ ./install.sh`
 2. Source `SOURCE_ME`: `$ . SOURCE_ME`
 3. That's it.

You may source `SOURCE_ME` after every time you log in again or put it in your `.bashrc` to do it automatically.


## Usage

### Overview




## TODOs

 - Replacing the function extractor to a nicer version.

`extfunc` is a script responsible for extracting functions from source code. The current implementation is quite _horrible_, to say the least, riddled with a ton of heuristics and even Linux-specific corner case handling. While I wouldn't bother to change it if it's working fine, but unfortunately, it isn't sometimes; it only works decently well for the Linux kernel code, and sometimes it takes forever if the source size is enormous.

However, replacing it is pretty straightforward. We can either improve or rewrite the script or, if we're not bothered to use another tool, we can even use Clang's AST generation, extract the ASTs of individual functions, and _pretty-print_ it to convert back ASTs to source code. All these things are engineering work, but hey. It should be fun. (weekend project?)

 - Exposing tunable parameters to a config file.

Some tunable parameters are sprinkled across the scripts as hard-coded numbers (e.g., the risk/weight bound), as there is no good enough reason why they _should_ take on the values they have now. It'd be great to set these parameters using a tidy config file.

 - Optimizing the usage of Joern.

Currently, the scripts are simply invoking the command line Joern _every time_ it matches a new batch of functions. The problem is the initialization time of the command line Joern is pretty prohibitable, and I don't think it's even necessary. I tried to utilize the workspace reset feature instead of launching the entire Joern again, but it was not entirely resetting Joern, leaking memory, and eventually killed by OOM.

This could have been fixed in the recent Joern version, but I'm not sure the newer version could introduce another problem (Joern can sometimes be unstable in an unexpected way; patterns are usable in one version while they aren't in another version). If not, we can tweak Joern to have a (sort of) _forkserver_ facility that launches a new clean-slate Joern instance right after the initialization.
