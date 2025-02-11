GITROOT  ?= $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
GITHASH  ?= $(shell git log -1 --format=%H)
GITREV   ?= $(shell git rev-list --count $(GITHASH))
NUMCORES ?= 4

VER_CFG  ?= $(shell cat $(GITROOT)/build-files/ver.cfg)
VERSION  ?= $(VER_CFG).$(GITREV)

nothing:
	@echo "The default target will do nothing. Please use one of the defined targets"

do_replacements:
	find . -name '*pro' | while read line ; do echo "VERSION=__QGVDIAL_VERSION__" >> $$line ; done
	perl ./build-files/version.pl __QGVDIAL_VERSION__ $(VERSION) .
	perl ./build-files/version.pl __MY_MIXPANEL_TOKEN__ $(shell cat ./secrets/mixpanel.token | tr -d '\n') .
	perl ./build-files/version.pl __THIS_IS_MY_EXTREMELY_LONG_KEY_ $(shell cat ./secrets/cipher_qgvdial | tr -d '\n') .
	perl ./build-files/version.pl __QT5_BB10__ '\/home\/admin\/bin\/qt5\/armle' .

# Using Crave
crave_full:
	./qgvdial/features/dbus_api/gen/create_ifaces.sh
	mkdir -p build/ubuntu_x86_64
	cd build/ubuntu_x86_64 ; qmake ../../qgvdial/qt-not-qml/desktop_linux.pro ; make -j

############################### dev x86_64 #################################
dev_enter:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt6 \
		/bin/bash

dev_make:
	mkdir -p $(GITROOT)/build/dev
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt6 \
		bash -c 'cd /tmp/src/qgvdial/features/dbus_api/gen ; ./create_ifaces.sh'
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt6 \
		bash -c 'cd /tmp/src/build/dev ; qmake ../../qgvdial/qt-not-qml/desktop_linux.pro'
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt6 \
		make -C /tmp/src/build/dev -j 4

############################### x86_64 #################################
qgvdial_ubuntu_x86_64:
	$(MAKE) -j$(NUMCORES) \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86_64 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		DEBMAKE=build-files/qgvdial/ubuntu \
		build

qgvdial_ubuntu_x86_64_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt6 \
		make -C /tmp/src \
		qgvdial_ubuntu_x86_64

qgvdial_ubuntu_x86_64_nodeb:
	$(MAKE) -j$(NUMCORES) \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86_64 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		build

qgvdial_ubuntu_x86_64_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt6 \
		make -C /tmp/src \
		qgvdial_ubuntu_x86_64_nodeb

build/mac/Makefile:
	mkdir -p build/mac
	cd build/mac ; qmake ../../qgvdial/qt-not-qml/desktop_mac.pro

qgvdial_mac: build/mac/Makefile
	make -C build/mac