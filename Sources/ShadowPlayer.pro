#-------------------------------------------------
#
# Project created by QtCreator 2014-04-30T21:32:45
#
#-------------------------------------------------

QT       += core gui winextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ShadowPlayer
TEMPLATE = app


SOURCES += main.cpp\
        shadowplayer.cpp \
    player.cpp \
    lyrics.cpp \
    lrcbar.cpp \
    spslider.cpp \
    playlist.cpp \
    MyGlobalShortCut/MyGlobalShortCut.cpp \
    MyGlobalShortCut/MyWinEventFilter.cpp \
    osd.cpp \
    fftdisplay.cpp \
    shadowlabel.cpp

HEADERS  += shadowplayer.h \
    player.h \
    tags.h \
    lyrics.h \
    lrcbar.h \
    spslider.h \
    playlist.h \
    MyGlobalShortCut/MyGlobalShortCut.h \
    MyGlobalShortCut/MyWinEventFilter.h \
    osd.h \
    fftdisplay.h \
    shadowlabel.h \
    ID3v2Pic.h

FORMS    += shadowplayer.ui \
    lrcbar.ui \
    playlist.ui \
    osd.ui

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

LIBS += -L$$PWD/ -lbass
PRE_TARGETDEPS += $$PWD/bass.lib
LIBS += -L$$PWD/ -ltags
PRE_TARGETDEPS += $$PWD/tags.lib
LIBS += -L$$PWD/ -lbass_ape
PRE_TARGETDEPS += $$PWD/bass_ape.lib
LIBS += -L$$PWD/ -lbass_aac
PRE_TARGETDEPS += $$PWD/bass_aac.lib
LIBS += -L$$PWD/ -lbass_alac
PRE_TARGETDEPS += $$PWD/bass_alac.lib
LIBS += -L$$PWD/ -lbass_tta
PRE_TARGETDEPS += $$PWD/bass_tta.lib
LIBS += -L$$PWD/ -lbassflac
PRE_TARGETDEPS += $$PWD/bassflac.lib
LIBS += -L$$PWD/ -lbasswma
PRE_TARGETDEPS += $$PWD/basswma.lib
LIBS += -L$$PWD/ -lbasswv
PRE_TARGETDEPS += $$PWD/basswv.lib
LIBS += -L$$PWD/ -lbass_fx
PRE_TARGETDEPS += $$PWD/bass_fx.lib


RESOURCES += \
    res.qrc

OTHER_FILES += \
    ShadowPlayer.rc

RC_FILE = ShadowPlayer.rc
