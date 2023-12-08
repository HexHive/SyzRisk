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

 - [Replacing the function extractor to a nicer version.](./todo/extfunc.md)
 - [Exposing tunable parameters to a config file.](./todo/param.md)
 - [Optimizing the usage of Joern.](./todo/joern.md)
 - [Improving the database management.](./todo/db.md)
