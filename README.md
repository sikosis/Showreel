<h1 align="center">showreel</h1>

<p align="center">
  <img src="https://img.shields.io/badge/version-v0.49-f5a623" alt="Version v0.49">
  <img src="https://img.shields.io/badge/platform-Haiku-ffcc00" alt="Platform: Haiku">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599c" alt="C++17">
</p>

Showreel is a native Haiku app for presenting a project's source files as a
slow-scrolling slideshow in Globe. The current build recursively discovers text
and source files, skips hidden and build-output folders, and fills the playlist
in a stable folder-first order. Resource definition (`.rdef`) files are treated
as primary project files, followed by header files and then C/C++ implementation
files. Markdown and project-automation scripts remain visible but are greyed out
and skipped during playback. Play now opens each viewable file in Globe, dwells
at the top, fits and centres the Globe window within the screen, scrolls it,
dwells at the bottom, closes it and advances. Pause, Previous, Next and Stop
control the active run. Top dwell, scrolling speed, bottom dwell and presentation
font size are adjustable before playback; the top and bottom dwell defaults are
five seconds and the presentation font defaults to 16 pt.
The playback status reports dwell countdowns, scroll progress, and files that
fit without scrolling. Globe closes when the playlist finishes, and Showreel
can optionally close too. Globe starts in automation mode without an empty
Untitled window. Pressing Escape during playback stops the slideshow, closes
its Globe document and returns to ShowReel. Showreel opens near the top-left of
the screen.

## Command-line automation

Every playback setting can be supplied when launching Showreel. A project may
be given positionally or with `--project`:

```sh
Showreel --play --top-dwell 5 --scroll-speed 8 --bottom-dwell 5 --font-size 16 \
	--close /boot/home/projects/showreel
```

Run `Showreel --help` for the accepted ranges and all options. Without command-
line arguments, Showreel retains the same interactive defaults.

Monitor mode can also be started from the command line:

```sh
Showreel --monitor /boot/home/projects/showreel
```

While Monitor Mode is enabled, ShowReel checks the selected project for changed
viewable files. Each change is allowed to settle for five seconds, then that
file is presented with the normal dwell and scrolling sequence. ShowReel
returns to monitoring when the file finishes. Files removed or renamed during
the settle delay are discarded quietly as normal build activity. Normal Play is
disabled while monitoring, so unchanged playlist files are never presented.

## Tracker add-on

`make install` installs both ShowReel and an **Open in ShowReel** Tracker
add-on. Right-click a project folder in Tracker and choose the add-on to open
that folder in ShowReel. Playback remains stopped so its settings can be
reviewed before pressing Play. The packaged HPKG includes the same add-on.

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
