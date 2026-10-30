<h1 align="center">showreel</h1>

<p align="center">
  <img src="https://img.shields.io/badge/version-v0.14-f5a623" alt="Version v0.14">
  <img src="https://img.shields.io/badge/platform-Haiku-ffcc00" alt="Platform: Haiku">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599c" alt="C++17">
</p>

Showreel is a native Haiku app for presenting a project's source files as a
slow-scrolling slideshow in Pe. The first build provides the application shell,
project-folder chooser, playlist area and playback controls. File discovery and
Pe automation are the next milestones described in [DESIGN.md](DESIGN.md).

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
package. The Tracker and Pe scripting spike in the design must also be run on
Haiku before playback automation is implemented.
