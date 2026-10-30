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
WARNINGS = ALL
COMPILER_FLAGS = -std=c++17
LOCALES =

VERSION = 0.12
PACKAGE_REVISION = 1
INSTALL_DIR ?= /boot/home/config/non-packaged/apps
PACKAGE_BUILD_DIR = package-build
PACKAGE_ARCHITECTURE = $(CPU)
PACKAGE_FILE = showreel-$(VERSION)-$(PACKAGE_REVISION)-$(PACKAGE_ARCHITECTURE).hpkg

include /boot/system/develop/etc/makefile-engine

.PHONY: install package package-clean

install: $(TARGET)
	mkdir -p "$(DESTDIR)$(INSTALL_DIR)"
	cp -f "$(TARGET)" "$(DESTDIR)$(INSTALL_DIR)/$(NAME)"
	@if [ -z "$(DESTDIR)" ]; then mimeset -f "$(INSTALL_DIR)/$(NAME)"; fi

package: $(PACKAGE_FILE)

$(PACKAGE_FILE): $(TARGET) packaging/PackageInfo.in
	rm -rf "$(PACKAGE_BUILD_DIR)"
	mkdir -p "$(PACKAGE_BUILD_DIR)/apps"
	cp -f "$(TARGET)" "$(PACKAGE_BUILD_DIR)/apps/$(NAME)"
	sed \
		-e 's/@VERSION@/$(VERSION)/g' \
		-e 's/@REVISION@/$(PACKAGE_REVISION)/g' \
		-e 's/@ARCHITECTURE@/$(PACKAGE_ARCHITECTURE)/g' \
		packaging/PackageInfo.in > "$(PACKAGE_BUILD_DIR)/.PackageInfo"
	rm -f "$@"
	package create -C "$(PACKAGE_BUILD_DIR)" "$(abspath $@)"

package-clean:
	rm -rf "$(PACKAGE_BUILD_DIR)" "$(PACKAGE_FILE)"

clean: package-clean
