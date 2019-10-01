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

- [x] audio.c
	- [x] Tidy up
	- [x] Add DAC capabilities
        - [x] Simulate audio using SDL

- [x] acodecs component: component containing all decoders
        - [x] abstract decoder component
	- [x] stb_vorbis
	- [x] dr_mp3
        - [x] mod, xm, it: libxmp-lite
        - [x] dr_wav
        - [x] dr_flac

- [ ] audio_player.c:
	- [x] fix audio_shutdown leaves speaker making noise
	- [x] mp3 support
	- [x] display/sdcard solve spi_bus race condition
	- [x] Show DAC/Speaker output mode in player
	- [x] Refactor to be more sane with player state etc
	- [x] Next/last track in folder
	    - [x] automatically
            - [x] dpad
            - [x] what happens if song ended
        - [x] Show some guide on player
        - [x] Error handling for bad audio files

- [ ] Status bar
    - [ ] Speaker/DAC
    - [ ] Battery meter

- [ ] Rename to ogo-shell
- [ ] Release?
    - [x] Change license to GPL
    - [ ] Update README.md
            - [ ] Instructions
            - [ ] Screenshots/GIF?

v0.3 - Polish and file management
---------------------------------

- [ ] UI/Window System:
        - [ ] Windows/Apps
        - [ ] Event bubbling
- [ ] Figure out a plan for error handling that allows displaying on lcd
- [ ] Icons instead of f and d
	- [ ] Create icon font
		- [ ] File, Directory, Music File
		- [ ] Battery, Headphones, Speaker, Settings, ...
		- [ ] Buttons (A) (B) for Help texts
- [ ] Remember old selection_pos when going back
- [ ] Wrap around selection if hitting boundaries?

- [ ] Audio Player++
        - [ ] New Theme?
        - [ ] Seeking
        - [ ] Show Song Position/Song Length

- [ ] Bigger Cleanup/Refactor
	- [ ] Menu as ui_list?
	- [ ] Dialogs?
	- [ ] Smaller but static functions

- [ ] START -> Action Dialog
	- [ ] Details
- [ ] SELECT -> Select/Mark files
	- [ ] Show selection as different background color

Ideas for future versions(maybe)
--------------------------------

- [ ] More Audio codecs
    - [ ] Use [gme](http://slack.net/~ant/libs/audio.html) to play gameboy/amiga/nes tunes
    - [ ] AAC - License Isuses?
    - [ ] OPUS - Library small and fast enough?

- [ ] Bluetooh Audio Streaming
- [ ] Musical Alarm Clock with NTP Time Sync
- [ ] Make a filesytem on flash with some demo content

- [ ] Show sdcard statistics using [getfree](http://elm-chan.org/fsw/ff/doc/getfree.html)
- [ ] Act as an emulator launcher to launch rom files
- [ ] Maybe even become a firmware that can install and launch *.fw files

- [ ] Odroid API Work?
	- [ ] Better error handling
	- [ ] Simulation
	- [ ] Docs?
