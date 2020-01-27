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
- VOLUME: Switch DAC/Speaker
- SELECT: Toggle display backlight for longer battery life?
- START: Toggle Playlist playing mode
- UP: Increase volume
- DOWN: Decrease volume
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

- [x] audio_player.c:
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

- [x] Status bar
	- [x] Battery meter

- [x] Save DAC/Volume and AudioPlayer settings to NVM

- [x] Rename to ogo-shell
- [x] Release
	- [x] Change license to GPL
	- [x] Update README.md
		- [x] Instructions
		- [x] Screenshots/GIF?
	- [x] Build and upload firmware
	- [x] Move all known issues and features to GitHub?

v0.3 - Image viewer and Keylock
-------------------------------

- [x] Simple image viewer(for small images)
- [x] Keylock when screen is off?
- [x] Update guide for start/select
- [x] Song repeat mode
- [x] Toggle fullscreen in imageviewer using select
- [x] Update readme
- [x] Release

v0.4 - Chiptunes and optional Emulators
---------------------------------------

- [x] Use [gme](http://slack.net/~ant/libs/audio.html) to play gameboy/atari/nes tunes
	- [x] Implement all the other formats/file endings
	- [x] Adapt README.md
	- [ ] m3u/multi track support
- [ ] Optional emulators
	- [x] Document building
	- [ ] Adapt colorsheme and center on screen menu?
	- [ ] Add confirmation dialog when quitting without saving state
- [ ] Bugfixes and minor improvements
        - [ ] Mono file playback
        - [x] Remember old selection_pos when going back
        - [ ] Show longer filenames when filesize is hidden

v1.0 - Polishing/UI Revamp
--------------------------

- [ ] Bookmark function
- [ ] polished odroid-go api/drivers: ogo-hal
        - [ ] Better error handling
        - [ ] Simulation using SDL
        - [ ] Documentation using read-the-docs

- [ ] Speaker/DAC in Statusbar
- [ ] UI/Window System:
            - [ ] Windows/Apps
            - [ ] Event bubbling
- [ ] Figure out a plan for error handling that allows displaying on lcd
- [ ] Icons instead of f and d
	- [ ] Create icon font
		- [ ] File, Directory, Music File
		- [ ] Battery, Headphones, Speaker, Settings, ...
		- [ ] Buttons (A) (B) for Help texts

- [ ] Audio Player++
	- [ ] New Theme?
	- [ ] Seeking
	- [ ] Show Song Position/Song Length
	- [ ] Shuffle Playlist
	- [ ] Play .m3u/playlist files?
	- [ ] Minimize Player for multi tasking

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
	- [ ] AAC - License Isuses?
	- [ ] OPUS - Library small and fast enough?

- [ ] Optimize image loading to use less memory in order to open larger images

- [ ] Bluetooh Audio Streaming
- [ ] Musical Alarm Clock with NTP Time Sync
- [ ] Make a filesytem on flash with some demo content

- [ ] Show sdcard statistics using [getfree](http://elm-chan.org/fsw/ff/doc/getfree.html)
- [ ] Act as an emulator launcher to launch rom files
- [ ] Maybe even become a firmware that can install and launch *.fw files

