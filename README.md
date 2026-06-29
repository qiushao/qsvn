# qsvn

qsvn is a small Qt 6 SVN desktop client for Linux. It follows the main
TortoiseSVN workflows as a standalone application instead of a file manager
shell extension.

Current scope:

- Open an existing SVN working copy.
- Checkout a repository URL.
- Export a repository URL or working copy path.
- Import a local directory into a repository URL.
- Browse a repository URL with `svn list`.
- Create, copy, rename, and delete repository folders from the repository browser.
- Browse files in the working copy.
- Show `svn status`.
- Create branches or tags with `svn copy`.
- Switch a working copy to another repository URL.
- Relocate a working copy after the repository URL changes.
- Merge from another branch or URL into the current working copy.
- Run update, update-to-revision, add, ignore, copy, delete, rename, revert, cleanup, resolve, lock,
  unlock, diff, log, blame, and commit.
- Show a working-copy conflict list and launch conflict edit or resolve actions.
- Show revision history as a structured log table with changed paths.
- Show revision diffs from the structured log window.
- Run blame for a changed path at a selected log revision.
- Create and apply patch files.
- View, set, and delete SVN properties on selected paths.
- Assign and remove SVN changelists for selected paths.
- Configure SVN authentication arguments for non-interactive commands.
- Configure an external diff tool such as `meld {base} {working}`.
- Configure an external merge tool and launch it for conflicted files.
- Launch common actions from CLI arguments for file manager integration.
- Show command output in the main window.

The application uses the system `svn` executable as its backend.

## Build

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

## Run

```sh
./build/qsvn
```

Examples:

```sh
qsvn --open /path/to/working-copy
qsvn --update /path/to/working-copy
qsvn --commit /path/to/working-copy
qsvn --add /path/to/working-copy/new-file.cpp
qsvn --ignore /path/to/working-copy/generated.tmp
qsvn --copy /path/to/working-copy/file.cpp
qsvn --delete /path/to/working-copy/old-file.cpp
qsvn --rename /path/to/working-copy/file.cpp
qsvn --revert /path/to/working-copy/file.cpp
qsvn --cleanup /path/to/working-copy
qsvn --conflicts /path/to/working-copy
qsvn --diff /path/to/working-copy/file.cpp
qsvn --log /path/to/working-copy
qsvn --blame /path/to/working-copy/file.cpp
qsvn --properties /path/to/working-copy/file.cpp
qsvn --repo-browser
```

The project installs a KDE/Dolphin service menu prototype at
`share/kio/servicemenus/qsvn-dolphin.desktop`.

It also installs Nautilus script prototypes under
`share/qsvn/nautilus-scripts/qsvn`. To enable them for the current user, copy
that `qsvn` directory into `~/.local/share/nautilus/scripts/`.

For Thunar, the project installs a custom actions template at
`share/qsvn/thunar/uca.xml`. Merge the actions from that file into
`~/.config/Thunar/uca.xml` instead of overwriting an existing user file.
