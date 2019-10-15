#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := ogo-shell
VERSION := $(shell cat version)

include $(IDF_PATH)/make/project.mk

MKFW := mkfw
APP_FW:=$(APP_BIN:.bin=.fw)
APP_FW_EMUS:=$(APP_BIN:.bin=.emus.fw)
APP_FW_ZIP:=$(APP_FW:.fw=.fw.zip)

CPPFLAGS += -D APP_NAME=\"$(PROJECT_NAME)\" -D APP_VERSION=\"$(VERSION)\" -D ODROID_GO_SPI_MUTEX

dist_noemus: $(APP_FW) $(APP_FW_ZIP)
dist_emus: $(APP_FW_EMUS)

dist: dist_emus dist_noemus

$(APP_FW): $(APP_BIN)
	@echo MKFW $@
	@$(MKFW) "$(PROJECT_NAME)($(VERSION))" media/tile.raw 0 16 786432 "$(PROJECT_NAME)" $^
	@mv firmware.fw $@

$(APP_FW_EMUS): $(APP_BIN)
	tools/make_emus_fw.sh

$(APP_FW_ZIP): $(APP_FW)
	zip $@ $^
