DATESTR  := $(shell date +'%a, %d %b %Y %H:%M:%S %z')
GITROOT  := $(shell git rev-parse --show-toplevel)
VER_CFG  := $(shell cat $(GITROOT)/build-files/ver.cfg)
GITHASH  := $(shell git log -1 --format=%H)
GITREV   := $(shell git rev-list --count $(GITHASH))
VERSION  := $(VER_CFG).$(GITREV)
BUILDDIR := $(GITROOT)/builds

ABSPROPATH := $(shell readlink -f $(PROPATH))

help:
	@echo "Example usage:"
	@echo "make -f ./build-files/qgvdial/Makefile build TARGET=ubuntu_x86_64 PROPATH=./qgvdial/qt-not-qml/desktop_linux.pro"
	@echo "make -f ./build-files/qgvdial/Makefile build TARGET=ubuntu_x86    PROPATH=./qgvdial/qt-not-qml/desktop_linux.pro"
	@echo "make -f ./build-files/qgvdial/Makefile build TARGET=maemo         PROPATH=./qgvdial/maemo/maemo.pro"
	@echo "make -f ./build-files/qgvdial/Makefile build TARGET=harmattan     PROPATH=./qgvdial/harmattan/harmattan.pro"

build: prep_build_target
	cd $(BUILDDIR)/$(TARGET)/qgvdial-$(VERSION) ; qmake $(ABSPROPATH)
	cd $(BUILDDIR)/$(TARGET)/qgvdial-$(VERSION) ; $(MAKE)

clean:
	# Set TARGET to "ubuntu" / "sailfish" / etc
	rm -rf $(BUILDDIR)/$(TARGET)

prep_build_target:
	mkdir -p $(BUILDDIR)/$(TARGET)/qgvdial-$(VERSION)

test:
	@echo $(ABSPROPATH)
