# This is an application
TEMPLATE=app

# The binary name
TARGET=phonetiq

# This app uses Qtopia
CONFIG+=qtopia
DEFINES+=QTOPIA

# I18n info
STRING_LANGUAGE=en_US
LANGUAGES=en_US

HEADERS=\
    src/phonetiq.h \
    src/JarScanner.h

SOURCES=\
    src/main.cpp \
    src/phonetiq.cpp \
    src/JarScanner.cpp

# Package info
pkg [
    name=phonetiq
    desc="MIDlet manager for PhoneME (Java ME Virtual Machine)"
    license=GPLv3
    version=1.0
    maintainer="Anton Olkhovik <ant007h@gmail.com>"
]

target [
    hint=sxe
    domain=untrusted
]

desktop [
    hint=desktop
    files=phonetiq.desktop
    path=/apps/Applications
]

pics [
    hint=pics
    files=pics/*
    path=/pics/phonetiq
]

help [
    hint=help
    source=help
    files=*.html
]
