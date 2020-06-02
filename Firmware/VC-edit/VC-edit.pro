#############################################################################
##
## Copyright (C) 2018 Catrinus Feddema.
## All rights reserved.
##
## This file is part of "VC-edit".
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
##
#############################################################################

# Compiling options are set in Headers/VController/config.h
# To compile the editor for the full VController, add // before the line #define IS_VCMINI
# To compile the editor for the VC-mini, remove the // before the line //#define IS_VCMINI

QT       += core gui

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wno-missing-field-initializers

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VC-edit
TEMPLATE = app

SOURCES += main.cpp\
    RtMidi.cpp\
    mainwindow.cpp \
    midi.cpp \
    vcsettings.cpp \
    customcombobox.cpp \
    customspinbox.cpp \
    vceditsettingsdialog.cpp \
    vccommands.cpp \
    commandeditdialog.cpp \
    VController/config.cpp \
    VController/globals.cpp \
    VController/globaldevices.cpp \
    devices/device.cpp \
    devices/gp10.cpp \
    devices/gr55.cpp \
    devices/vg99.cpp \
    devices/zg3.cpp \
    devices/zms70.cpp \
    devices/m13.cpp \
    devices/helix.cpp \
    devices/fractal.cpp \
    devices/katana.cpp \
    customlistwidget.cpp \
    aboutdialog.cpp \
    customslider.cpp \
    devices/kpa.cpp \
    vcmidiswitchsettings.cpp \
    vcdevices.cpp \
    devices/svl.cpp \
    custompatchlistwidget.cpp \
    devices/sy1000.cpp \
    devices/gmajor2.cpp

HEADERS  += RtMidi.h \
    mainwindow.h \
    midi.h \
    vcsettings.h \
    customcombobox.h \
    customspinbox.h \
    vceditsettingsdialog.h \
    vccommands.h \
    commandeditdialog.h \
    VController/leds.h \
    VController/hardware.h \
    VController/config.h \
    VController/globals.h \
    VController/globaldevices.h \
    devices/device.h \
    devices/gp10.h \
    devices/gr55.h \
    devices/vg99.h \
    devices/zg3.h \
    devices/zms70.h \
    devices/m13.h \
    devices/helix.h \
    devices/fractal.h \
    devices/katana.h \
    customlistwidget.h \
    aboutdialog.h \
    customslider.h \
    devices/kpa.h \
    vcmidiswitchsettings.h \
    vcdevices.h \
    devices/svl.h \
    custompatchlistwidget.h \
    devices/sy1000.h \
    devices/gmajor2.h

FORMS += \
    vceditsettingsdialog.ui \
    commandeditdialog.ui \
    mainwindow_VC-full.ui \
    aboutdialog_VC-full.ui \
    aboutdialog_VC-mini.ui \
    mainwindow_VC-mini.ui

RESOURCES += \
    myresources.qrc

DISTFILES +=

macx{
    DEFINES += __MACOSX_CORE__=1
    LIBS += -framework CoreMidi
    LIBS += -framework CoreAudio
    LIBS += -framework CoreFoundation
    ICON = VC-edit_icon.icns
}

linux{
    DEFINES += define __LINUX_ALSA__=1
    LIBS += -lasound
}
win32{
    DEFINES += __WINDOWS_MM__=1
    LIBS += -lwinmm
    RC_FILE = VC-edit_icon.rc

    CONFIG += static
    CONFIG += embed_manifest_exe
    CONFIG += release
    DESTDIR = ./packager
    MOC_DIR += ./generatedfiles/release
    OBJECTS_DIR += release
    UI_DIR += ./generatedfiles
    INCLUDEPATH += ./generatedfiles \
        ./generatedfiles/release \
}


