# 29 · Every command

Every command you type while working on the engine or the game, in one page, from installing the
tools to opening a pull request. Details are on the linked pages.

There are three kinds of configuration, each changed in its own place:

| When | What it decides | Where you change it |
|---|---|---|
| **Build time** | compiler, debug or release, dynamic or static, tests | a CMake preset (`cmake --preset …`) and CMake options (`-D…`) |
| **Start time** | role, game, plugins, port, window, log level, first scene | the YAML config next to the executable, overridden by command-line options |
| **Run time** | volumes, filters, debug drawing, network settings, cheats | the in-game console (F1) and console variables (CVars) |

## 1 · Install the tools (once per machine)

Linux (Ubuntu 24.04):

```bash
sudo apt install -y build-essential g++-14 libstdc++-14-dev clang-18 cmake ninja-build git \
    libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev \
    autoconf autoconf-archive automake libtool pkg-config clang-format clang-tidy
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh -disableMetrics
echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.zshrc       # or ~/.bashrc; then open a new terminal
```

Windows (PowerShell, with Visual Studio 2022 and its "Desktop development with C++" workload):

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat -disableMetrics
setx VCPKG_ROOT C:\vcpkg                                  # then open a new terminal
```

Use the "x64 Native Tools Command Prompt for VS 2022", or open the folder in Visual Studio or VS Code:
both load the MSVC environment the `windows-msvc-*` presets need.

## 2 · Build time: presets and options

```bash
cmake --list-presets                        # what this machine can use
cmake --preset linux-gcc-debug              # configure (first time: vcpkg builds the libraries, ~10 min)
cmake --build --preset linux-gcc-debug      # compile
ctest --preset linux-gcc-debug              # run the tests
```

| Preset | Platform | Mode |
|---|---|---|
| `linux-gcc-debug`, `linux-gcc-release` | Linux, GCC | dynamic (`.so`) |
| `linux-clang-debug`, `linux-clang-release` | Linux, Clang | dynamic |
| `windows-msvc-debug`, `windows-msvc-release` | Windows, MSVC | dynamic (`.dll`) |
| `linux-gcc-static`, `windows-msvc-static` | Linux, Windows | static (one executable) |

Options, added after a preset (`cmake --preset linux-gcc-debug -DENGINE_SANITIZERS=ON`):

| Option | Default | Effect |
|---|---|---|
| `ENGINE_DYNAMIC_MODULES` | ON | plugins and game as `.so` / `.dll`; OFF links everything into one executable |
| `ENGINE_WARNINGS_AS_ERRORS` | OFF (ON in CI) | a warning fails the build |
| `ENGINE_SANITIZERS` | OFF | AddressSanitizer and UndefinedBehaviorSanitizer (Linux) |
| `ENGINE_BUILD_TESTS` | ON | build `engine/tests` |
| `ENGINE_BUILD_BENCHMARKS` | OFF | build `engine/benchmarks` |

Other useful ones:

```bash
cmake --build --preset linux-gcc-debug --target engine     # only the engine library
cmake --build --preset linux-gcc-debug --clean-first       # rebuild everything
rm -rf build/linux-gcc-debug                               # start the configuration from scratch
cmake -S engine -B build/engine-alone -G Ninja             # the engine alone, without the game (slide 14)
```

Personal presets (another compiler path, extra flags) go in `CMakeUserPresets.json`, ignored by Git.

## 3 · Start time: running and command-line options

```bash
./build/linux-gcc-debug/bin/r-type_server                         # headless server, port from server.yaml
./build/linux-gcc-debug/bin/r-type_client                         # client window
./build/linux-gcc-debug/bin/r-type_client --platform raylib       # another backend, same build (Part 2)
./build/linux-gcc-debug/bin/r-type_server --port 5000 --log debug
./build/linux-gcc-debug/bin/r-type_client --set r_debug_colliders=1 --set snd_music=0.3
./build/linux-gcc-debug/bin/r-type_client --platform null --audio null --ticks 1000   # headless smoke test
```

Every option and every key of the YAML file: [02 · Project and configuration](02-project-and-config.md).
Exit codes: same page.

## 4 · Run time: console and CVars

Open the console with **F1**. Tab completes, up and down browse the history.

```text
help                      net                     systems
set r_color_filter deuteranopia                   system disable rtype.boss_ai
get snd_master            cvars net_              inspect player_1
bind Fire Space           scene scenes/level2.yaml quit
```

Every command and every engine CVar: [13 · Console, CVars and metrics](13-console-cvars-metrics.md).
`Archive` variables are saved in the user data folder (`~/.local/share/<game>/settings.yaml`,
`%APPDATA%\<game>\settings.yaml`).

## 5 · Installing the engine and shipping

```bash
cmake --install build/linux-gcc-release --prefix ~/rtype-engine    # headers, libraries, find_package files
cpack --preset linux-gcc-release                                   # r-type-<version>-linux.tar.gz
```

Another project then uses `find_package(rtype_engine REQUIRED)`: [24 · Building and shipping](24-building-and-shipping.md).

## 6 · Code style and checks

```bash
clang-format -i engine/src/ecs/Registry.cpp                         # format one file (editors do it on save)
git ls-files 'engine/*.hpp' 'engine/*.cpp' | xargs clang-format --dry-run --Werror   # what CI checks
clang-tidy -p build/linux-gcc-debug engine/src/ecs/Registry.cpp    # static analysis, warnings only
```

## 7 · Project documentation and tasks

```bash
python3 docs/tasks/generate.py                     # regenerate task pages, boards, folder tree, skeletons
python3 docs/tasks/notion_export.py ENG-01 SRV-02  # Notion content of tasks (JSON), to update the boards
```

Run `generate.py` after any change to `docs/tasks/*.py` or to a reference page that declares classes;
it never overwrites code below a skeleton's header.

## 8 · Git workflow

Branches: `main` → `develop` → `engine`, `server`, `game` → `feature/<area>-<feature>`.

```bash
git switch engine && git pull
git switch -c feature/engine-ecs                   # or switch to the existing feature branch
git add -p && git commit -m "ENG-16: entity allocator with generations"
git push -u origin feature/engine-ecs
gh pr create --base engine --title "Engine: ECS (ENG-16 to ENG-21)"   # when the feature is done
```

Before claiming a task, set it on the Notion board: status, owner, claim date and deadline. A pull
request goes to `engine` (or `server`, `game`) when the CI is green; `develop` receives those branches,
and `main` receives `develop` for a defense.
