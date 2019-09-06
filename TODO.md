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
- A: Open selected file/
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

- [ ] Commit
- [ ] Cleanup
- [ ] Check for memory leaks
- [ ] Deploy test on ogo
	- [ ] Initialize sdcard
	- [ ] Larger stacksize

- [ ] Minimal sdkconfig and fw/app size

## Later

- [ ] Remember old selection_pos when going back

- [ ] Odroid API Work?
	- [X] endianess as bool
	- [ ] Better error handling
	- [ ] Simulation
	- [ ] Docs?

