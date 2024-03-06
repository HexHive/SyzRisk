### Improving the database management.

`data` is the directory that collects and caches every analysis detail for later usage. We have `rc` for the ground-truth dataset, `funcs` for extracted function bodies, `match` for matched functions, `risk` for estimated risks, and `repo_path` for the path to the current target directory. There are some obvious problems here and there.

First, it heavily utilizes the directory hierarchy to manage data. For example, `match` contains the directories that represent each commit, each of which contains the directories corresponding to each function. This structure makes sense and actually comes in handy for scripting, but the nightmare starts when you ever want to manually inspect it or move it somewhere else; it takes way more time to handle a huge number of directories (both from you and your system) when the actual data is actually not that big. One potential improvement is making it _not_ to (ab)use the directory structure.

Second, it avoids re-analyzing a portion of Git history if it was done before (that's what caching is all about). That's good, but it only keeps track of _one range of time_ to identify whether a certain commit or function was analyzed; if it falls into this time range, the script just _assumes_ it was analyzed. Various problems can arise from this. What if you analyzed _multiple_ ranges of time? What if you introduced new patterns after the last analysis? Keeping track of multiple ranges of time and the matched patterns in each range may solve this issue.