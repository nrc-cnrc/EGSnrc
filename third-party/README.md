# Third-party contributions to EGSnrc

Users and developers outside the National Research Council Canada contribute source code to the EGSnrc project. In some cases, these contributions are integrated in the EGSnrc source code directly. In other cases, it makes more sense to include the third-party project as a `submodule` inside the `EGSnrc/third-party/` directory. A submodule allows authors to develop software in their own repository, under their own terms, as an otherwise independent project. Note that you must have installed EGSnrc with `git` in order to obtain third-party software in the way described below.

_**Important: we do not provide support for content inside `EGSnrc/third-party/`. For any issue, suggestion, bug, question, etc., please contact the third-party software author.**_

## Submodules

Submodules solve the issue of including a git repository inside another one (which is not possible otherwise). When cloning the EGSnrc repository, submodules show up as standard subdirectories under `EGSnrc/third-party/`, but these directories are all empty initially. At this point, submodules are simply *links* to other projects that users might find useful (technically, these links are stored in `.gitmodules`). In other words, third-party software is **not  distributed as part of EGSnrc.** However, you can choose to fetch the *content* of any individual submodule of interest.

### Getting a submodule

To download the content of a submodule, use the `git submodule` command. For example, you can get the `3ddose_tools` submodule with the command:

```bash
git submodule update --init 3ddose_tools
```

This populates the `EGSnrc/third-party/3ddose_tools/` directory with that software's source code, downloaded from the third-party `3ddose_tools` repository location.

### Using submodules

The submodule author is responsible for providing instructions to install and run their software, possibly involving some additional steps to make it work with your existing EGSnrc installation. Look for a `README.md` file inside the submodule directory.

### Updating submodules

When the upstream submodule software is updated, for example a new version of `3ddose_tools` is released, you can bring your copy up to date with the command:

```
git submodule update 3ddose_tools
```

Note that when updating EGSnrc with `git pull`, submodules are not updated by default, and `git` might warn you that some submodules are out of date. You must run the command above manually in order to update a submodule.

### Useful git settings

Two *global* git config settings prove particularly useful when working with submodules:

```bash
git config --global diff.submodule log
git config --global status.submodulesummary 1
```

The first provides a nicer diff output for submodule differences (listing the new commits for each submodule); the second adds this submodule diff summary at the end of the git status command.

### Running git commands in a submodule

Inside the submodule directory, `git` commands pertain to the submodule repository, not EGSnrc. For example, if you run `git log` inside `EGSnrc/third-party/3ddose_tools/`, you will see the commit history of that project, not that of EGSnrc. Likewise, you can run other `git` commands as usual. However, it is best to avoid adding commits to the submodule default branch to prevent confusion when updating the submodule.

Note that the EGSnrc repository itself does **not** contain the submodule files, only a link and the current commit hash for each submodule. Therefore, running `git diff` (outside the submodule directory) reports submodule differences in terms of commit hashes, not in terms of file content. To review changes to the submodule *files,* run `git diff` *inside* the submodule directory.