# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

qsvn is a Qt 5 (C++17) SVN desktop client for Linux. It is a standalone GUI that mirrors the
TortoiseSVN workflows without being a file-manager shell extension. It shells out to the system
`svn` executable as its backend — there is no libsvn linkage. All SVN operations are built as
`svn` argument lists, run via `QProcess`, and the textual/XML output is parsed back into structs.

## Build / run / test

```sh
cmake -S . -B build -G Ninja
cmake --build build          # produces build/qsvn and build/svnclient_tests
./build/qsvn                 # run the GUI
cmake --build build --target svnclient_tests   # build tests alone
ctest --test-dir build                       # run tests via ctest
```

Run a single test (QtTest binary, names are exact test-slot names):

```sh
./build/svnclient_tests parseStatusReadsCodesAndPaths
```

The test binary `svnclient_tests` is only built when `BUILD_TESTING` is on (default via `include(CTest)`)
and `Qt5::Test` is found. Many `runHandles*` tests create a real on-disk repo with `svnadmin create`
inside a `QTemporaryDir` and drive it through `svn` — they `QSKIP` if `svn`/`svnadmin` are not on PATH,
so the suite still passes on a machine without a subversion client. The pure parser tests
(`parseStatus*`, `parseLogXml*`, `parsePropertiesXml*`, `buildArguments*`, `parseCommandLine*`,
`buildExternalToolCommand*`) need no `svn` binary.

Install (system-wide service menus + binary):

```sh
cmake --install build
```

## Architecture

The codebase is a single `src/` tree compiled into one `qsvn` executable, plus one QtTest binary.
There is no separation into libraries. Key layers:

- **`SvnClient`** (`src/SvnClient.{h,cpp}`) — the backend. Holds `SvnOptions` (auth) and exposes
  `buildArguments()`, `run()`, and three output parsers: `parseStatus` (text), `parseLogXml` (XML),
  `parsePropertiesXml` (XML). `run()` always injects `--non-interactive` (unless already present) and
  appends auth flags from options; it locates `svn` via `QStandardPaths::findExecutable`. `SvnResult`
  is the uniform return type (`ok()` = started && !timedOut && exitCode==0); `combinedOutput()` merges
  stdout+stderr for display. The displayed command line masks `--password` values.

- **`MainWindow`** (`src/MainWindow.{h,cpp}`, ~2000 lines) — the entire UI and orchestration. Owns an
  `SvnClient m_svn` and `ApplicationSettings m_settings`. Pattern for every operation: build a
  `QStringList` of `svn` subcommand args, call `runWorkingCopyCommand(args, refreshAfter)` (or
  `loadStatus` for status views), then `appendResult(result)` echoes the command+output into the
  `m_outputView` panel and a failure pops `QMessageBox::warning`. Public `*Path(const QString&)`
  slots are the CLI entry points (called from `main.cpp`); private `*Selected()`/`*WorkingCopy()`
  slots are wired to menu/toolbar `QAction`s in `createActions()`. `updateActionState()` toggles
  action availability based on selection/working-copy state.

- **Dialogs** (`src/*Dialog.{h,cpp}`) — one `QDialog` subclass per workflow (Checkout, Commit, Log,
  RepositoryBrowser, Properties, Conflicts, Branch/Tag, Merge, Switch, Relocate, Export, Import,
  Settings). Several emit a `commandFinished(SvnResult)` signal that `MainWindow::appendResult`
  listens to, so a dialog can run its own `svn` command and still log into the main output panel.

- **`CommandLine`** (`src/CommandLine.{h,cpp}`) — `parseCommandLine()` maps `qsvn --<action> <path>`
  flags to a `CommandLineRequest{action, path, error}`. A bare path (no `--`) is treated as `--open`.
  `main.cpp` dispatches each `CommandLineAction` to the matching `MainWindow::*Path()` slot via a
  `QTimer::singleShot(0, ...)` so the window is shown before the action runs.

- **`ApplicationSettings`** (`src/ApplicationSettings.{h,cpp}`) — persistence wrapper over `QSettings`
  (org/app = "qsvn"). Stores `svn/*` auth keys and `tools/external{Diff,Merge}Command`. `MainWindow`
  pushes these into `m_svn` as `SvnOptions` and into `buildExternalToolCommand()` placeholders.

- **`ExternalToolCommand`** (`src/ExternalToolCommand.{h,cpp}`) — parses a user-configured diff/merge
  command line via `QProcess::splitCommand`, substitutes `{placeholder}` tokens, and if no placeholder
  was used appends default args (so `meld` works bare and `meld {base} {working}` works templated).

### File-manager integration

`packaging/` holds prototypes (not built) installed by CMake: a KDE/Dolphin service menu
(`packaging/kde/qsvn-dolphin.desktop`), Nautilus scripts (`packaging/nautilus/qsvn/*`), and a Thunar
custom-actions template (`packaging/thunar/uca.xml`). Each just invokes `qsvn --<action> <path>`.
Adding a new GUI action usually means adding a matching CLI flag + Nautilus script + desktop entry.

## Conventions

- Adding a new SVN operation: add the arg list + call site in `MainWindow`, add a `*Path` public slot
  + `CommandLineAction` enum value + `parseCommandLine` branch + `main.cpp` dispatch case, and a
  parser test plus (if it exercises a new `svn` flow) a `runHandles*` integration test in
  `tests/svnclient_tests.cpp`. New `svn` output shapes get a `parse*` method in `SvnClient` and a
  fixture-based test (see `parseStatusReadsCodesAndPaths` for the style).
- All `svn` invocations go through `SvnClient::run()` — do not spawn `svn` directly elsewhere, or the
  `--non-interactive`/auth masking invariants break.
- Use `QStringLiteral` for string literals (matches existing style); `#pragma once` for headers.
