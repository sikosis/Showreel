<h1 align="center">showreel</h1>

<p align="center">
  <img src="https://img.shields.io/badge/version-v0.2-f5a623" alt="Version v0.2">
  <img src="https://img.shields.io/badge/platform-Haiku-ffcc00" alt="Platform: Haiku">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599c" alt="C++17">
</p>

Showreel is a native Haiku app for presenting a project's source files as a
slow-scrolling slideshow in Globe. The current build recursively discovers text
and source files, skips hidden and build-output folders, and fills the playlist
in a stable folder-first order. Resource definition (`.rdef`) files are treated
as primary project files and appear before ordinary files in their folder. Play
opens the selected file in Globe; automated scrolling is the next milestone
described in [DESIGN.md](DESIGN.md).

## Building

On Haiku with the development tools installed:

```sh
make
make install
make package
```

The build uses Haiku's standard `makefile-engine`. `make install` installs to
`/boot/home/config/non-packaged/apps` by default; override `INSTALL_DIR` to use
another location. `make package` creates a versioned `.hpkg` in the repository
root. `make clean` removes the build, package staging directory and generated
package. The Tracker and Globe scripting spike in the design must also be run
on Haiku before playback automation is implemented.
