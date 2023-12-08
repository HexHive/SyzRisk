# SyzRisk Pre-fuzzing Components

The pre-fuzzing components of SyzRisk, including:

 1. Ground-truth dataset collection (for Linux)
 2. Pattern development
 3. Risk estimation
 4. Weight generation

The recommended usage roughly follows the same order, too.

## Summary

 0. Install.
```
$ ./install.sh
$ . SOURCE_ME
```

 1. Set the path to your (Git-managed) target project.
```
$ kfc-repo <dir/path/to/proj>
``` 

 2. Match patterns to the target's commit history.
```
$ kfc-match -d <yyyy-mm-dd>:<yyyy-mm-dd>
```

 3. Estimate pattern risks.
```
$ kfc-risk
```

 4. Generate weights.
```
$ kfc-weight   # produces a weight profile in JSON.
```

<details>
<summary>Click here for details.</summary>
 
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

### 1. Setting the target project path

### 2. Matching patterns

### 3. Estimating pattern risks

### 4. Generating weights


## Developing custom patterns


## TODOs

 - [Replacing the function extractor to a nicer version.](./todo/extfunc.md)
 - [Exposing tunable parameters to a config file.](./todo/param.md)
 - [Optimizing the usage of Joern.](./todo/joern.md)
 - [Improving the database management.](./todo/db.md)

</details>
