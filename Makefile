NAME = Showreel
TYPE = APP
APP_MIME_SIG = application/x-vnd.Sikosis-Showreel
SRCS = \
	src/Main.cpp \
	src/ShowreelApplication.cpp \
	src/ShowreelWindow.cpp
RDEFS = resources/Showreel.rdef
LIBS = be tracker
OPTIMIZE := FULL
LOCAL_CFLAGS = -std=c++17
LOCALES =

include /boot/system/develop/etc/makefile-engine
