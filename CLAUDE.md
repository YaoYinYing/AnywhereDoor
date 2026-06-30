# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

AnywhereDoor is a shell-integrated proxy management tool for Linux and macOS. Users `source` `anywhere_door.sh` into their shell (bash/zsh), which defines an `anywhere_door` function for switching SOCKS5/HTTP proxies within a terminal session. The core logic is in C (v2.0.0-alpha); the original Python implementation is archived in `retired/python/`.

## Build / test / lint

- **Build**: `cd src/anywheredoor && cmake -B build && cmake --build build`
- **Release build**: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`
- **Dependencies**: libyaml, libcurl, ncurses (all linked dynamically). macOS: `brew install libyaml curl ncurses cmake`. Linux: `apt install libyaml-dev libcurl4-openssl-dev libncurses-dev cmake`.
- **C standard**: C11
- **Compiler**: Clang (macOS) or GCC (Linux)
- There is no test suite and no linting configuration.

## Architecture: Shell + C hybrid with an eval boundary

The central design challenge is that some commands must modify the calling shell's environment variables (`http_proxy`, `https_proxy`, `all_proxy`), while others just produce output. This is solved by splitting commands across two execution paths in `anywhere_door.sh`:

### Shell layer (`anywhere_door.sh`)

The `anywhere_door` Bash function uses a `case` statement to route every subcommand into one of three categories:

| Category | Commands | Mechanism |
|---|---|---|
| **Direct** | `test`, `dns`/`leak`, `help`/`?`, `show`, `list`/`ls`, `git`, `docker_daemon`, `export`, `use` (no args), `tui` | Runs C binary directly — output goes to terminal. |
| **Eval** | `on`, `off`, `config`, `use` (with args) | Runs `eval $(C_binary <args> 2>/dev/null)` — C binary prints `export`/`unset` statements to stdout (status messages go to stderr); shell evaluates them. |
| **Pure shell** | `version`, `gost`, `bench`, `wget`, `curl`, `whereami`, `ipq`, `netq`, `upgrade`, `refresh` | No C binary involved. `upgrade` fetches the latest prebuilt binary from GitHub Releases via curl. |

The shell script auto-locates the C binary by checking: `$ANYWHERE_DOOR_DIR/build/anywhere_door`, `$ANYWHERE_DOOR_DIR/src/anywheredoor/build/anywhere_door`, `/usr/local/bin/anywhere_door`, and `$PATH`. ANSI colors are inlined in the script (no external dependency). Tab-completion is defined in `_anywhere_door_completions`.

### C layer (`src/anywheredoor/`)

```
utils.h/c        ANSI colors, dynamic string builder (StrBuf), path resolution, run_command()
    ↑
proxy.h/c        ProxyConfig, ProxyTable, NoProxy structs + expand/flatten logic
    ↑
config.h/c       YAML parsing via libyaml event-based API; state-machine parser
    ↑
test.h/c         Concurrent proxy testing via libcurl multi interface
    ↑
dnsleak.h/c      DNS leak test via bash.ws API (libcurl HTTP + system ping)
    ↑
tui.h/c          ncurses interactive terminal UI (proxy list, status, color coding)
    ↑
main.c           CLI dispatcher — argc/argv parsing, routes to command handlers
CMakeLists.txt   Build config: C11, find libyaml/libcurl/ncurses, link executable;
                 auto-extracts VERSION from anywhere_door.sh (unless -DVERSION= override)
```

**Version chain**: `anywhere_door.sh` (`export ANYWHERE_DOOR_DIR_VERSION=...`) is the single source of truth outside `src/`. CMake reads it automatically — `cmake -B build` produces a binary with that version. CI passes `-DVERSION=...` to append the short git SHA. The C code has a `#ifndef VERSION` / `#define VERSION "unknown"` safety fallback for non-CMake builds only.

**`#include <stdint.h>`** is required in `test.c` for `intptr_t` — Linux GCC does not transitively include it unlike macOS Clang.

**Key data structures** (`proxy.h`):
- `ProxyConfig`: url, ports, auth, alternative_urls array, test_urls, label. Functions: `proxy_http_url()`, `proxy_socks_url()`, `proxy_expand()` — clones one config per alt_url (suffixed `_0`, `_1`, etc.), `proxy_matches_env()`.
- `ProxyTable`: fixed-size array of `ProxyConfig*` pointers (max 256). `proxy_table_expand()` flattens all alt_urls into numbered list.
- `NoProxy`: string array formatted as semicolon-separated `NO_PROXY` env var.

**Eval output pattern**: Commands that modify the shell environment (`on`, `off`, `use`, `config`) print `export`/`unset` lines to stdout. Status messages go to stderr so they don't pollute the eval output. The shell wrapper runs `eval $(binary args 2>/dev/null)` to suppress stderr during eval.

**Concurrent testing** (`test.c`): Uses `curl_multi_perform()` + `curl_multi_wait()` for true async I/O — each proxy gets its own easy handle. Results collected via `CURLOPT_PRIVATE` index tracking.

## Configuration

`.anywheredoorrc` is YAML with two sections:
- `proxies`: List of proxy definitions (`label`, `url`, `http_port`, `socks_port`, `authentication_user`, `password`, `alternative_urls`).
- `no_proxy`: List of CIDR ranges / hostnames to bypass the proxy.

Lookup priority: `./.anywheredoorrc` → `~/.anywheredoorrc` → `$ANYWHERE_DOOR_DIR/.anywheredoorrc`.

## CI / Releases

`.github/workflows/release.yml` builds on 7 platforms:

| Platform | Runner | Method |
|---|---|---|
| `linux-x86_64` | `ubuntu-latest` | Native GCC |
| `linux-aarch64` | `ubuntu-24.04-arm` | Native ARM64 |
| `linux-armv7` | `ubuntu-latest` | QEMU/Docker (`ubuntu:24.04`) |
| `linux-i386` | `ubuntu-latest` | QEMU/Docker (`debian:bookworm` — Ubuntu dropped 32-bit) |
| `linux-riscv64` | `ubuntu-latest` | QEMU/Docker (`ubuntu:24.04`) |
| `darwin-arm64` | `macos-latest` | Native Clang |
| `darwin-x86_64` | `macos-latest` | Cross-compiled from arm64 via `-DCMAKE_OSX_ARCHITECTURES=x86_64` |

All Docker builds run `sudo chown` after the container to fix root-owned build directories. On tag `v*`, binaries are attached to a GitHub Release. Actions are at latest major versions (checkout@v7, upload-artifact@v7, download-artifact@v8, setup-qemu-action@v4).

## Vendored external tools

`tools/IPQuality/` and `tools/NetQuality/` are vendored third-party Bash scripts called by `anywhere_door ipq` and `anywhere_door netq`. They are opaque subprocess invocations.

## Retired Python code

The original Python implementation is preserved in `retired/python/` for reference. It is not used by the current version.
