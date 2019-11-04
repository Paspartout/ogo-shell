#!/bin/sh

set -e

JOBS=$(nproc)
GO_PLAY_PATH=../go-play-pelle
MKFW=mkfw

PROJECT_NAME='ogo-shell'
VERSION="$(cat version)"

GNUBOY_PATH=$GO_PLAY_PATH/gnuboy-go
NESEMU_PATH=$GO_PLAY_PATH/nesemu-go
SMSPLUSGX_PATH=$GO_PLAY_PATH/smsplusgx-go

APP_BIN=build/ogo-shell.bin
GNUBOY_BIN=$GNUBOY_PATH/build/gnuboy-go.bin
NESEMU_BIN=$NESEMU_PATH/build/nesemu-go.bin
SMSPLUSGX_BIN=$SMSPLUSGX_PATH/build/smsplusgx-go.bin

(cd $GNUBOY_PATH && make -j $JOBS)
(cd $NESEMU_PATH && make -j $JOBS)
(cd $SMSPLUSGX_PATH && make -j $JOBS)

make -j $JOBS

$MKFW "${PROJECT_NAME}(${VERSION})" media/tile.raw 0 16 1048576 "$PROJECT_NAME" ${APP_BIN} 0 17 786432 nesemu ${NESEMU_BIN} 0 18 786432 gnuboy ${GNUBOY_BIN} 0 19 1572864 smsplusgx ${SMSPLUSGX_BIN}
mv firmware.fw build/ogo-shell.emus.fw

