#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := ogo-shell
VERSION := 0.2.0

include $(IDF_PATH)/make/project.mk

MKFW := mkfw
APP_FW:=$(APP_BIN:.bin=.fw)

CPPFLAGS += -D APP_NAME=\"$(PROJECT_NAME)\" -D APP_VERSION=\"$(VERSION)\"

dist: $(APP_FW)

$(APP_FW): $(APP_BIN)
	@echo MKFW $@
	@$(MKFW) "$(PROJECT_NAME)($(VERSION))" media/tile.raw 0 16 524288 "$(PROJECT_NAME)" $^
	@mv firmware.fw $@
