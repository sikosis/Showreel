NAME = Showreel
TYPE = APP
APP_MIME_SIG = application/x-vnd.ShowReel
SRCS = \
	src/EditorDriver.cpp \
	src/GlobeDriver.cpp \
	src/Main.cpp \
	src/SourceFileDiscovery.cpp \
	src/ShowreelApplication.cpp \
	src/ShowreelWindow.cpp
RDEFS = resources/Showreel.rdef
LIBS = be tracker $(STDCPPLIBS)
OPTIMIZE := FULL
WARNINGS = ALL
CXXFLAGS += -std=c++17
LOCALES =

VERSION = 0.5
PACKAGE_REVISION = 1
INSTALL_DIR ?= /boot/home/config/non-packaged/apps
PACKAGE_BUILD_DIR = package-build
PACKAGE_ARCHITECTURE = $(CPU)
PACKAGE_FILE = showreel-$(VERSION)-$(PACKAGE_REVISION)-$(PACKAGE_ARCHITECTURE).hpkg
TRACKER_ADDONS_DIR ?= /boot/home/config/non-packaged/add-ons/Tracker
TRACKER_ADDON_NAME = Open in ShowReel

include /boot/system/develop/etc/makefile-engine

.PHONY: package package-clean

install::
	mkdir -p "$(TRACKER_ADDONS_DIR)"
	copyattr --data -- "$(TARGET)" "$(TRACKER_ADDONS_DIR)/$(TRACKER_ADDON_NAME)"

package: $(PACKAGE_FILE)

$(PACKAGE_FILE): $(TARGET) packaging/PackageInfo.in packaging/Copyrighted
	rm -rf "$(PACKAGE_BUILD_DIR)"
	mkdir -p "$(PACKAGE_BUILD_DIR)/apps"
	mkdir -p "$(PACKAGE_BUILD_DIR)/add-ons/Tracker"
	mkdir -p "$(PACKAGE_BUILD_DIR)/data/licenses"
	cp -f "$(TARGET)" "$(PACKAGE_BUILD_DIR)/apps/$(NAME)"
	ln -s ../../apps/$(NAME) "$(PACKAGE_BUILD_DIR)/add-ons/Tracker/$(TRACKER_ADDON_NAME)"
	cp -f packaging/Copyrighted "$(PACKAGE_BUILD_DIR)/data/licenses/Copyrighted"
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
