# R-Type and its engine

A networked R-Type built on our own C++23 game engine. The engine is generic: the game is a module
it loads, rendering, audio, network and scripting are plugins, and the server runs the same engine
without a window.

> **Status (Part 1).** The engine runs end to end: launcher, configuration, plugins (SFML, miniaudio,
> Asio), ECS, scenes and prefabs, systems, input, rendering, audio, network session and replication,
> with 277 tests on Linux and Windows. The handshake between a real client and a real server
> (SRV-01 to SRV-03) and the R-Type game itself are in progress.

## Requirements

| | Linux | Windows |
|---|---|---|
| Compiler | GCC 14+ or Clang 18+ | Visual Studio 2022 (MSVC 17.10+) |
| Build | CMake 3.25+, Ninja | CMake 3.25+ (comes with Visual Studio) |
| Packages | [vcpkg](https://github.com/microsoft/vcpkg), `VCPKG_ROOT` set | same |
| System (Linux) | `libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev`, and `autoconf autoconf-archive automake libtool pkg-config` for vcpkg | — |

vcpkg installs every library listed in `vcpkg.json` (SFML, miniaudio, Asio, yaml-cpp, Catch2, …) on
the first configure.

## Build

```sh
git clone https://github.com/microsoft/vcpkg ~/vcpkg && ~/vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg

cmake --preset linux-gcc-debug          # or linux-clang-debug, linux-gcc-release
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug          # run the tests
```

On Windows, from a *Developer PowerShell for VS 2022*:

```powershell
git clone https://github.com/microsoft/vcpkg $HOME\vcpkg; & $HOME\vcpkg\bootstrap-vcpkg.bat
$env:VCPKG_ROOT = "$HOME\vcpkg"
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

`-static` presets (`linux-gcc-static`, `windows-msvc-static`) build everything into one executable
instead of loading plugins at run time. Binaries go to `build/<preset>/bin/`.

## Run

Each game ships a launcher and a YAML configuration. The test game shows the commands:

```sh
cd build/linux-gcc-debug/bin
./smoke_launcher --ticks 1000                       # client (null platform: no window), stops after 1000 ticks
./smoke_launcher --role server --port 4242          # server, no window, until Ctrl+C
./smoke_launcher --platform sfml --audio miniaudio  # client with a window and sound
```

Connecting a client to a server (`--host`, `--port`) works once the handshake (SRV-01 to SRV-03)
lands.

Options override the configuration file: `--config`, `--role client|server`, `--platform`,
`--audio`, `--network`, `--host`, `--port`, `--scene`, `--ticks N` (stop after N ticks),
`--log trace|debug|info|warn|error`. A wrong option prints the closest valid one and exits with
code 1.

## Documentation

Open `index.html` through any static server (`python3 -m http.server`, then
<http://localhost:8000>) or read the Markdown files directly in `docs/`. The site is also published
by the `docs` workflow.

| Start here | |
|---|---|
| [Engine architecture](docs/engine/architecture.md) | the big picture |
| [Engine guide](docs/engine/guide/README.md) | making a game with the engine, page by page |
| [Engine reference](docs/engine/reference/README.md) | every class and method |
| [Protocol (MSEP RFC)](docs/network/rfc.md) | every byte on the wire |
| [Decision records](docs/adr/README.md) | why each choice was made |
| [Technical and comparative study](docs/study/README.md) | libraries, algorithms, storage, security, accessibility |
| [Legal and privacy](docs/study/legal-and-privacy.md) | personal data, licences |
| [Changelog](docs/CHANGELOG.md) | what changed, by date |

## Repository

```
engine/        the engine: include/ (public API), src/, plugins/, launcher/, tests/, benchmarks/
docs/          documentation (Markdown) and docs/index.json for the site
index.html     documentation site (renders docs/)
vcpkg.json     dependencies       CMakePresets.json   build presets
```

## Team

Ola, Sedjro, Michel and Yann-Ange (Epitech, G-ING-500 R-Type, 2026). Work is tracked on the
team's task boards; each task has a page in [docs/tasks](docs/tasks/README.md).
