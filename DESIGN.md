# Showreel: A Code Showcase App for Haiku OS

Oct 30, 2026 · @Sikosis

## Overview

Showreel is a small Haiku app that plays a project's source code like a slideshow: it opens each text or source file in Globe, scrolls it slowly top to bottom, pauses at the end, closes it and moves to the next. It is meant for demos, screen recordings and kiosk-style display of an app's code.

The automation runs on Haiku's messaging and scripting systems. Tracker shows the project folder, while Globe provides the document and scrolling surface. Showreel sends native BMessages where possible and keeps `hey` useful for testing the public scripting contract.

**Goals**

- Point it at a folder, press Play, and it walks every source file with no further input.
- Smooth, readable scrolling at a pace the user sets.
- Pause, skip, previous and stop at any time without leaving stray Globe windows behind.

**Non-goals (v1)**

- Editing files, syntax-theme management or recording video; screen recording is left to other tools.
- Supporting editors other than Globe in v1, though the driver layer keeps that possible later.

## How it works

Each showcase run is one setup step followed by a loop that repeats once per file. Every step is a scripting message, so the app never needs to click or type into another app.

1. **Show the project.** Ask Tracker to open the chosen folder so the audience sees the file layout first.
2. **Build the playlist.** Walk the folder, keep text and source files, and sort them in a fixed order.
3. **Open a file.** Send the file reference to Globe through its application signature.
4. **Identify the document.** Use Globe's automation API to obtain a stable document identifier for the Showreel-owned window.
5. **Scroll slowly.** Move the text view down in small timed steps until the end.
6. **Dwell.** Wait at the bottom for a set time so the last lines can be read.
7. **Close and advance.** Ask Globe to close that document, then go back to step 3 with the next file.

| Step | Intended mechanism | Confidence |
| --- | --- | --- |
| Show folder in Tracker | `hey Tracker` scripting, or `open <folder>` as a fallback | Needs a spike on a Haiku machine |
| Open file in Globe | Send a refs-received message to `application/x-vnd.hGlobe` | Implemented |
| Identify new document | Globe returns a stable document/session identifier | Needs Globe API |
| Scroll | Set or advance the document's exposed scroll position | Needs Globe API |
| Close window | Close the identified Showreel-owned Globe document | Needs Globe API |

I have not run these against a live Haiku install, so the exact property names are the first thing to confirm (see the build plan).

## Architecture

&#91;embedded content: architecture · 5 components, 2 external apps\]

The player never talks to Tracker or Globe directly; it asks the driver layer, which uses native messages and the public scripting contract so a backend can be swapped later.

## Key behaviours

**File discovery.** Recurse from the chosen folder, skip hidden entries, `.git`, build output and binaries. Include a file if its MIME type starts with `text/` or its extension is in an editable source list (`.cpp`, `.h`, `.c`, `.rdef`, `.html`, `.js`, `Jamfile`, `Makefile` and similar). Markdown and project-automation scripts remain visible in a disabled style but playback skips them. Default order is folders first, then resource definitions, headers, C/C++ implementations and the remaining files by name; the user can drag rows in the playlist to reorder.

**Slow scrolling.** The scroller computes the document's scrollable range once, then advances on a timer, for example every 50 ms. Speed is set in lines per second, so long files take longer and short files finish quickly. A file that fits in one window skips scrolling and just dwells.

**Dwell at the bottom.** When the scroll bar reaches its maximum, the app waits
for the configured dwell time (default 5 s), then closes the window. A dwell at
the top (default 5 s) lets the first lines register before scrolling starts.

**Monitor mode.** When enabled, ShowReel rescans the selected project for
changes once per second. A changed or newly discovered viewable file is queued
for five seconds, allowing saves to settle, then presented with the normal top
dwell, scrolling and bottom dwell sequence. Repeated changes restart the
five-second delay. After that one file closes, ShowReel returns to monitoring.
Normal playlist playback is disabled while Monitor Mode is active.
Content fingerprints determine whether files changed, so timestamp-only updates
do not queue unchanged files again.

**Closing safely.** The app only closes Globe documents opened with its session identifier. Showreel requests read-only presentation mode, so its documents cannot become modified and do not produce save prompts.

**Emergency stop.** Pressing Escape in Globe's presentation window sends a
stop request back to ShowReel. ShowReel closes its current Globe document,
stops playback and returns its control window to the foreground.

**Controls.** Pause freezes the timer; Next and Previous close the current window and jump; Stop closes the window and returns to the playlist. A loop option restarts from the first file after the last.

## UI and settings

One small native window (Haiku Interface Kit) with a playlist on the left and the controls on the right. It stays out of the way so Globe fills the screen.

| Setting | Default | Range | Purpose |
| --- | --- | --- | --- |
| Project folder | none | any folder | Root for discovery and the Tracker window |
| Scroll speed (lines/s) | 8 | 2 to 40 | How fast text moves |
| Dwell at top (s) | 5 | 0 to 10 | Pause before scrolling |
| Dwell at bottom (s) | 5 | 0 to 30 | Pause on the last lines |
| Presentation font (pt) | 16 | 10 to 28 | Temporary Globe font size |
| Monitor mode | off | on or off | Present changed files after a five-second delay |
| File filter | text and source | editable list | Which files join the playlist |
| Loop | off | on or off | Restart after the last file |
| Globe window | maximised | maximised or as is | Whether to resize Globe for the show |

Settings are saved with `BMessage` flattening to the user's settings directory. A per-file row shows its state (queued, playing, done, skipped) so the user can see where the run is.

## Risks and build plan

The main integration task is defining a small, stable Globe automation contract for document ownership and scrolling.

| Risk | Effect | Fallback |
| --- | --- | --- |
| Globe automation API is not yet available | Cannot complete playback | Add the document/session and scrolling properties to Globe |
| Document identity is not stable | Wrong window closed | Require an opaque identifier returned when Showreel opens a document |
| Unsaved-changes prompt on close | Run stalls | Detect the prompt, stop and warn; open files read-only where possible |
| Globe already has other windows open | Closing the wrong window | Only touch documents carrying Showreel's session identifier |
| `hey` call latency | Choppy scrolling | Send scroll messages natively; keep `hey` for open, find and close |

**Build plan**

- [ ] Add and document Globe's read-only document/session and scrolling API.
- [ ] Prove the Globe loop by hand: open a file, identify it, scroll and close it.
- [ ] Build the driver layer with two backends, `hey` and native BMessenger, behind one interface.
- [x] Build discovery and populate the playlist model.
- [x] Build the player state machine (idle, scrolling, dwelling and advancing).
- [x] Build the initial native window shell.
- [ ] Add settings and connect the window to the playlist and player.
- [ ] Test on a large project, a one-line file, an empty file and a file that needs saving.

**Open questions**

- Should Showreel also drive a second editor (for example Pe) in later versions?
- Should Globe's presentation mode hide its chrome for cleaner recordings?
