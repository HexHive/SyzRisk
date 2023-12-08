# SyzRisk Pre-fuzzing Components

The pre-fuzzing components of SyzRisk, including:

 1. Root-cause commit collection (for Linux)
 2. Pattern development
 3. Risk estimation
 4. Weight generation

The recommended usage roughly follows the same order, too.

## System Requirement

 - OS: Ubuntu 20.04
 - CPU: Intel(R) i7-8665
 - Memory: 16GB

Other OSes _may_ work well without problems, especially on another Linux distro. Smaller memory space can be problematic with some parts (e.g., pattern matching during risk estimation and weight generation).

## Build

 1. Run `install.sh`: `$ ./install.sh`
 2. Source `SOURCE_ME`: `$ . SOURCE_ME`
 3. That's it.

## Usage

### Overview


## TODOs

 - Replacing the function extractor to a nicer version.

`extfunc` is a script that's responsible for extracting functions from source code. The current implementation is quite _horrible_, to say the least, riddled with a ton of heuristics and even Linux-specific corner case handling. While I wouldn't bother to change it if it's working fine, but unfortunately, it isn't sometimes; it only works decently well for the Linux kernel code, and sometimes it takes forever if the source size is huge.

However, the good news is that replacing it is quite straightforward. We can either improve or rewrite the script or, if we're not bothered to use another tool, we can even use Clang's AST generation, extract the ASTs of individual functions, and _pretty-print_ it to convert back ASTs to source code. All these things are engineering work, but hey. It should be fun. (weekend project?)

 - Exposing tunable parameters to a config file.

Some tunable parameters are sprinkled across the scripts as hard-coded numbers (e.g., the risk/weight bound), as there is no good enough reason why they _should_ take on the values that they have now. It'd be great to set these parameters using a tidy config file.
