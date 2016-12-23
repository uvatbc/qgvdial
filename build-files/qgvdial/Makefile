DATESTR  := $(shell date +'%a, %d %b %Y %H:%M:%S %z')
GITROOT  := $(shell git rev-parse --show-toplevel)
VER_CFG  := $(shell cat $(GITROOT)/build-files/ver.cfg)
GITHASH  := $(shell git log -1 --format=%H)
GITREV   := $(shell git rev-list --count $(GITHASH))
VERSION  := $(VER_CFG).$(GITREV)
BUILDDIR := $(GITROOT)/builds

ABSPRO := $(shell readlink -f $(PRO))

help:
	@echo "Run make inside the appropriate container or build environment"
	@echo "Example usage:"
	@echo "make -f ./build-files/qgvdial/Makefile build FOR=ubuntu_x86_64 PRO=./qgvdial/qt-not-qml/desktop_linux.pro DEBMAKE=build-files/qgvdial/ubuntu"
	@echo "make -f ./build-files/qgvdial/Makefile build FOR=ubuntu_x86    PRO=./qgvdial/qt-not-qml/desktop_linux.pro DEBMAKE=build-files/qgvdial/ubuntu"
	@echo "make -f ./build-files/qgvdial/Makefile build FOR=harmattan     PRO=./qgvdial/harmattan/harmattan.pro      DEBMAKE=build-files/qgvdial/harmattan"
	@echo "make -f ./build-files/qgvdial/Makefile build FOR=maemo         PRO=./qgvdial/maemo/maemo.pro              DEBMAKE=build-files/qgvdial/harmattan"

build: prep_build_target
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; qmake $(ABSPRO)
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; $(MAKE)
ifdef DEBMAKE
	$(MAKE) -C $(GITROOT)/$(DEBMAKE) build BUILDDIR=$(BUILDDIR) FOR=$(FOR) VERSION=$(VERSION)
endif

clean:
	# Set FOR to "ubuntu" / "sailfish" / etc
	rm -rf $(BUILDDIR)/$(FOR)

prep_build_target:
	mkdir -p $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)
	./qgvdial/features/dbus_api/gen/create_ifaces.sh

test:
	@echo $(ABSPRO)
