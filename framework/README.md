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


## Developing Custom Patterns

Diff-level and function-level patterns reside in
`pipeline/anal{diff,func}/matcher`, respectively. Upon startup, the matcher
engines scan corresponding directories and conveniently recognize all available
pattern descriptions for you. All you need to do is to place a new pattern
description in the directory.

### Diff-level Pattern

 1. Create a subdirectory under `analdiff/matcher` named after your pattern.
    Let's say the pattern is called `skraa`.

```
$ cd analdiff/matcher
$ mkdir skraa
```

 2. Prepare the directory. For now, it only require one `__init__.py` file and
    one symbolic link called `common` that points to the `common` directory at
    the root.

```
$ touch __init__.py
$ ln -s ../../../../common common
```

 3. Steal a pattern description from another pattern for a starting point.
    `chained_deref` is so simple, it might as well be a de-facto template.

```
$ cp ../chained_deref/main.py .
```

 4. Specify `NAME`, `SHORT_NAME`, AND `DESCRIPTION`. Not all of them are in
    active use for the matcher engine now, but it's still good for clarification
    purposes. 

 5. Implement callback functions. You can use whatever Python package you want.
    I recommend to refer to other pattern descriptions for example.

    - `OnAnalysisBegin()`: called once upon the startup of the matcher engine.
    - `OnCommitBegin()`: called at the beginning of a commit. You might want to
      reset some of state variables for your description here.
    - `OnDiffLine()`: called when the matcher engine hit a new diff line.
        - `line`: the code subject to this diff.
        - `scope_type`: the scope of this code. (i.e., `func`, `struct`, `init`, or `enum`)
        - `scope_name`: the name of the scope. (e.g., function name)
        - `diff_type`: the type of this diff. (i.e., `+` or `-`)
    - `OnCommitEnd()`: called at the end of a commit. **This should return a set
      of the name of matched functions.**
    - `OnAnalysisEnd()`: called at the end of the matcher engine.

### Function-level Pattern

Like diff-level patterns, new function-level patterns will be automatically
recognized by the matcher engine as long as they reside in `analfunc/matcher`.
Function-level pattern descriptions are written in Scala and incorporate
[Joern](https://joern.io/) syntax, as the matcher engine uses it for matching
jobs.

 1. (Again) Steal one of existing matchers for a starting point. Let's steal
    `entering_goto.sc` this time and name it `papapa`.

```
$ cd analfunc/matcher
$ cp entering_goto.sc papapa.sc
```

 2. In a stolen description, rename the matcher name.

```scala
- object EnteringGotoMatcher extends Matcher {
+ object PapapaMatcher extends Matcher {
```

 3. Specify `name` and `attr`.
    - `name`: the name of this description (the short one).
    - `attr`: the list of intra-function analyses to request. It accepts
      whichever analysis Joern supports (check Joern for the list of supported
      analyses).

 4. Implement the matcher body (i.e., `Run()`). 
    - `method`: the current function under pattern matching.
    - `version`: the version of this method body (i.e., `old` or `new`).
    - `GetMetadata()`: (callable) returns corresponding metadata.
        - `ATTRS`: recognizable function attributes.
        - `HEXSHA`: hexsha of this commit.
    - **This should return True or False depending on whether the current
      function is matched or not.**

## TODOs

 - [Replacing the function extractor to a nicer version.](./todo/extfunc.md)
 - [Exposing tunable parameters to a config file.](./todo/param.md)
 - [Optimizing the usage of Joern.](./todo/joern.md)
 - [Improving the database management.](./todo/db.md)

</details>
