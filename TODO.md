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
- SELECT: Display file details

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

- [ ] Odroid API Work
	- [X] endianess as bool
	- [ ] Better error handling
	- [ ] Simulation
	- [ ] Docs?

- [ ] Setup vscode properly
	- [x] include paths
	- [ ] Makefile support
	- [ ] building + debugging simulation
	- [ ] building odroid go version?

- [ ] Create needed api overview for fs, draft program flow
	- [ ] Look at rover, nnn, noice and 3DShell for reference

- [ ] Implement GUI List with scrolling and single line selection
	- [ ] List == Array?
	- [ ] draw_list(gbuf, selection) ...

- [ ] Minimal sdkconfig and fw/app size