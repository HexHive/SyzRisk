### Optimizing the usage of Joern.

Currently, the scripts are simply invoking the command line Joern _every time_ it matches a new batch of functions. The problem is the initialization time of the command line Joern is pretty prohibitable, and I don't think it's even necessary. I tried to utilize the workspace reset feature instead of launching the entire Joern again, but it was not entirely resetting Joern, leaking memory, and eventually killed by OOM.

This could have been fixed in the recent Joern version, but I'm not sure the newer version could introduce another problem (Joern can sometimes be unstable in an unexpected way; patterns are usable in one version while they aren't in another). If not, we can tweak Joern to have a (sort of) _forkserver_ facility that launches a new clean-slate Joern instance right after the initialization.
