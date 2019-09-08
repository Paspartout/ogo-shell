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

v0.2 - Basic music playing capabilities
---------------------------------------

### Audio Player Dialog

#### Input:

- A: Play/Pause
- B: Close Song, Go back(Later: Continue playing in the background)
- START: Switch DAC/Speaker?
- SELECT: Toggle display backlight for longer battery life?
- VOLUME: Cycle volume
- RIGHT: Next Song
- LEFT: Previous Song

- [ ] audio.c
	- [x] Tidy up
	- [x] Add DAC capabilities
	- [ ] Understand differential output
	- [ ] Simulate audio using SDL
	- [ ] Even more tyding

- [ ] acodecs component: component containing all decoders
	- [x] stb_vorbis
	- [x] dr_mp3
	- [ ] dr_wav
	- [ ] dr_flac
	- [ ] mod, xm, it?

- [ ] audio_player.c:
	- [ ] fix audio_shutdown leaves speaker making noise
	- [ ] display/sdcard solve spi_bus race condition
	- [ ] mp3 support

- [ ] Backlight control
- [ ] Battery meter in top right corner with %
- [ ] Rename to ogo-shell
- [ ] Release?
	- [ ] Update README.md
		- [ ] Instructions
		- [ ] Screenshots

v0.3 - Polish and file management
---------------------------------

- [ ] Figure out a plan for error handling that allows displaying on lcd
- [ ] Icons instead of f and d
	- [ ] Create icon font
		- [ ] File, Directory, Music File
		- [ ] Battery, Headphones, Speaker, Settings, ...
		- [ ] Buttons (A) (B) for Help texts
- [ ] Remember old selection_pos when going back
- [ ] Wrap around selection if hitting boundaries?

- [ ] Bigger Cleanup/Refactor
	- [ ] Menu as ui_list?
	- [ ] Dialogs?
	- [ ] Smaller but static functions

- [ ] START -> Dialog
	- [ ] Details
- [ ] SELECT -> Select/Mark files
	- [ ] Show selection as different background color
- [ ] Understand and correct partitions.csv


Future versions
---------------

- [ ] Show sdcard statistics using [getfree](http://elm-chan.org/fsw/ff/doc/getfree.html)

- [ ] Odroid API Work?
	- [ ] Better error handling
	- [ ] Simulation
	- [ ] Docs?
