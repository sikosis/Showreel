# Showreel: A Code Showcase App for Haiku OS

Oct 30, 2026 · @Sikosis

## Overview

Showreel is a small Haiku app that plays a project's source code like a slideshow: it opens each text or source file in Pe, scrolls it slowly top to bottom, pauses at the end, closes it and moves to the next. It is meant for demos, screen recordings and kiosk-style display of an app's code.

The automation runs on Haiku's scripting system. `hey` (or the same BMessages sent natively) asks Tracker to show the project folder, then drives Pe window by window.

**Goals**

- Point it at a folder, press Play, and it walks every source file with no further input.
- Smooth, readable scrolling at a pace the user sets.
- Pause, skip, previous and stop at any time without leaving stray Pe windows behind.

**Non-goals (v1)**

- Editing files, syntax-theme management or recording video; screen recording is left to other tools.
- Driving editors other than Pe, though the driver layer keeps that possible later.

## How it works

Each showcase run is one setup step followed by a loop that repeats once per file. Every step is a scripting message, so the app never needs to click or type into another app.

1. **Show the project.** Ask Tracker to open the chosen folder so the audience sees the file layout first.
2. **Build the playlist.** Walk the folder, keep text and source files, and sort them in a fixed order.
3. **Open a file.** Hand the file's path to Pe, which opens it in a new window (or the running Pe instance).
4. **Find the window.** Use `hey` to list Pe's windows and match the new one by title.
5. **Scroll slowly.** Move the text view down in small timed steps until the end.
6. **Dwell.** Wait at the bottom for a set time so the last lines can be read.
7. **Close and advance.** Ask the Pe window to quit, then go back to step 3 with the next file.

| Step | Intended mechanism | Confidence |
| --- | --- | --- |
| Show folder in Tracker | `hey Tracker` scripting, or `open <folder>` as a fallback | Needs a spike on a Haiku machine |
| Open file in Pe | Pass the path to Pe (`Pe <file>`) or send it a refs-received message | High |
| Find new window | `hey Pe` GET on Window titles, compared with the file name | Medium |
| Scroll | Set the vertical scroll bar's value through `hey`, or send mouse-wheel messages natively | Medium; native fallback likely needed |
| Close window | `hey` quit on the window, or a native quit-requested message | Medium |

I have not run these against a live Haiku install, so the exact property names are the first thing to confirm (see the build plan).

## Architecture

&#91;embedded content: architecture · 5 components, 2 external apps\]

The player never talks to Tracker or Pe directly; it asks the driver layer, which can use `hey` or native messages, so a backend can be swapped if a property turns out not to be scriptable.

## Key behaviours

**File discovery.** Recurse from the chosen folder, skip hidden entries, `.git`, build output and binaries. Include a file if its MIME type starts with `text/` or its extension is in an editable list (`.cpp`, `.h`, `.c`, `.py`, `.rdef`, `.md`, `Jamfile`, `Makefile` and similar). Default order is folders first, then name; the user can drag rows in the playlist to reorder.

**Slow scrolling.** The scroller computes the document's scrollable range once, then advances on a timer, for example every 50 ms. Speed is set in lines per second, so long files take longer and short files finish quickly. A file that fits in one window skips scrolling and just dwells.

**Dwell at the bottom.** When the scroll bar reaches its maximum, the app waits for the configured dwell time (default 3 s), then closes the window. A short dwell at the top (default 1.5 s) lets the first lines register before scrolling starts.

**Closing safely.** The app only closes windows it opened, matched by title and the window's creation order. If Pe shows an unsaved-changes prompt, the app stops, tells the user, and does not force-quit.

**Controls.** Pause freezes the timer; Next and Previous close the current window and jump; Stop closes the window and returns to the playlist. A loop option restarts from the first file after the last.

## UI and settings

One small native window (Haiku Interface Kit) with a playlist on the left and the controls on the right. It stays out of the way so Pe fills the screen.

| Setting | Default | Range | Purpose |
| --- | --- | --- | --- |
| Project folder | none | any folder | Root for discovery and the Tracker window |
| Scroll speed (lines/s) | 8 | 2 to 40 | How fast text moves |
| Dwell at top (s) | 1.5 | 0 to 10 | Pause before scrolling |
| Dwell at bottom (s) | 3 | 0 to 30 | Pause on the last lines |
| File filter | text and source | editable list | Which files join the playlist |
| Loop | off | on or off | Restart after the last file |
| Pe window | maximised | maximised or as is | Whether to resize Pe for the show |

Settings are saved with `BMessage` flattening to the user's settings directory. A per-file row shows its state (queued, playing, done, skipped) so the user can see where the run is.

## Risks and build plan

The main risk is that Pe may not expose every property the loop needs through scripting, so the first task is a spike that proves each step by hand.

| Risk | Effect | Fallback |
| --- | --- | --- |
| Pe has no scriptable scroll position | Cannot scroll through `hey` | Send mouse-wheel messages natively to the text view |
| Window title does not match the file name | Wrong window closed | Match by window index before and after opening, and by path |
| Unsaved-changes prompt on close | Run stalls | Detect the prompt, stop and warn; open files read-only where possible |
| Pe already has other windows open | Closing the wrong window | Only touch windows created during the run |
| `hey` call latency | Choppy scrolling | Send scroll messages natively; keep `hey` for open, find and close |

**Build plan**

- [ ] Spike: on a Haiku machine, dump Tracker's and Pe's scripting suites with `hey` and write down which properties exist.
- [ ] Prove the loop by hand in a shell script: open a file, list windows, scroll, close.
- [ ] Build the driver layer with two backends, `hey` and native BMessenger, behind one interface.
- [x] Build discovery and populate the playlist model.
- [ ] Build the player state machine (idle, opening, scrolling, dwelling, closing).
- [x] Build the initial native window shell.
- [ ] Add settings and connect the window to the playlist and player.
- [ ] Test on a large project, a one-line file, an empty file and a file that needs saving.

**Open questions**

- Should Showreel also drive a second editor (for example StyledEdit) in later versions?
- Should it hide Pe's chrome for cleaner recordings, or leave Pe untouched?
