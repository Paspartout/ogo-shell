TODO
====

0.1 - File browsing
-------------------

No managing, just browsing the FS.

### Brainstorming

#### Input

- UP: Menu go up
- DOWN: Menu go down
- RIGHT: Next Page
- LEFT: Previous Page
- A: Go into directory/Show file details
- B: Go up a directory
- MENU: Go back to firmware
- START: Display file details

#### Display/Output

- Status Bar
	- Title + Version
	- Battery
- Path Bar
- Simple list
	- Rect with color depending on selection
	- File/Folder icon
	- File name

#### Tasks

- [X] Project strucutre

- [X] Make it buildable on the go
	- [X] Unified event handling: Implement wait_event() for go

- [X] Enable all warnings in simulation too

- [x] Setup vscode properly
	- [x] include paths
	- [x] Makefile support
	- [x] building + debugging simulation
	- [x] building odroid go version?

- [x] Figure out how to scroll many entries

- [x] Create needed api overview for fs, draft program flow
	- [x] Look at rover, nnn, noice and 3DShell for reference

- [x] file_ops.c: Main file browsing logic
	- [x] Entry/State structs
	- [x] ls()

- [x] file_browser.c: Uses file_ops and ui

- [x] Fix cdup
- [x] clear old entries on cding
- [x] show message on empty folders
- [x] Show current path somewhere
- [x] Show error messages as dialog
- [x] Show filesize?

- [x] Commit
- [x] Check for memory leaks and consumption
- [x] Deploy test on ogo
	- [x] Initialize sdcard
	- [x] Larger stacksize

- [x] Only stat using start
- [x] Show details on A and Start
- [x] Fix all warnings
- [x] Minimal sdkconfig and fw/app size
- [x] Format code

- [x] Commit and tag v0.1

v0.2 - Hacky music playing capabilities
---------------------------------------

- [ ] audio.c
	- [ ] Understand
	- [ ] Tidy up
	- [ ] Simulate using SDL?
	- [ ] Add DAC capabilities

- [ ] audio_dec component?
	- [ ] stb_vorbis
	- [ ] dr_mp3
	- [ ] dr_wav

- [ ] audio_player.c:
	- [ ] open(file_path)

- [ ] Backlight control
- [ ] Battery meter in top right corner with %
- [ ] Rename to ogo-shell

v0.3 - Polish and file management
---------------------------------

- [ ] Icons instead of f and d
- [ ] Remember old selection_pos when going back
- [ ] Wrap around selection if hitting boundaries?

- [ ] Bigger Cleanup/Refactor
	- [ ] Menu as ui_list?
	- [ ] Dialogs?
	- [ ] Smaller but static functions

- [ ] START -> Dialog
	- [ ] Details
- [ ] Understand and correct partitions.csv


Future versions
---------------

- [ ] Show sdcard statistics using [getfree](http://elm-chan.org/fsw/ff/doc/getfree.html)

- [ ] Odroid API Work?
	- [ ] Better error handling
	- [ ] Simulation
	- [ ] Docs?
