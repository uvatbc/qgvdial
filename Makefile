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
		accupara/qgvdial:qt5_amd64 \
		/bin/bash

dev_make:
	mkdir -p $(GITROOT)/build/dev
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt5_amd64 \
		bash -c 'cd /tmp/src/qgvdial/features/dbus_api/gen ; ./create_ifaces.sh'
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt5_amd64 \
		bash -c 'cd /tmp/src/build/dev ; qmake ../../qgvdial/qt-not-qml/desktop_linux.pro'
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt5_amd64 \
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
		accupara/qgvdial:qt5_amd64 \
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
		accupara/qgvdial:qt5_amd64 \
		make -C /tmp/src \
		qgvdial_ubuntu_x86_64_nodeb

############################### x86 #################################
qgvdial_ubuntu_x86:
	$(MAKE) -j$(NUMCORES) \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		DEBMAKE=build-files/qgvdial/ubuntu \
		build

qgvdial_ubuntu_x86_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt5_i386 \
		make -C /tmp/src \
		qgvdial_ubuntu_x86

qgvdial_ubuntu_x86_nodeb:
	$(MAKE) -j$(NUMCORES) \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		build

qgvdial_ubuntu_x86_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial:qt5_i386 \
		make -C /tmp/src \
		qgvdial_ubuntu_x86_nodeb

############################### maemo #################################
qgvdial_maemo_armel:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select FREMANTLE_ARMEL
	/scratchbox/login \
		make -j$(NUMCORES) -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo_armel \
			PRO=./qgvdial/maemo/maemo.pro \
			DEBMAKE=build-files/qgvdial/maemo \
			GITHASH=$(GITHASH) GITREV=$(GITREV) \
			build

qgvdial_maemo_armel_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/maemo \
		make -C /scratchbox/users/admin/home/admin/src/ qgvdial_maemo_armel

qgvdial_maemo_armel_nodeb:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select FREMANTLE_ARMEL
	/scratchbox/login \
		make -j$(NUMCORES) -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo_armel \
			PRO=./qgvdial/maemo/maemo.pro \
			GITHASH=$(GITHASH) GITREV=$(GITREV) \
			build

qgvdial_maemo_armel_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/maemo \
		make -C /scratchbox/users/admin/home/admin/src/ \
			GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
			qgvdial_maemo_armel_nodeb

qgvdial_maemo_x86:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select FREMANTLE_ARMEL
	/scratchbox/login \
		make -j$(NUMCORES) -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo_x86 \
			PRO=./qgvdial/maemo/maemo.pro \
			DEBMAKE=build-files/qgvdial/maemo \
			GITHASH=$(GITHASH) GITREV=$(GITREV) \
			build

qgvdial_maemo_x86_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/maemo \
		make -C /scratchbox/users/admin/home/admin/src/ qgvdial_maemo_x86

qgvdial_maemo_x86_nodeb:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select FREMANTLE_ARMEL
	/scratchbox/login \
		make -j$(NUMCORES) -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo_x86 \
			PRO=./qgvdial/maemo/maemo.pro \
			GITHASH=$(GITHASH) GITREV=$(GITREV) \
			build

qgvdial_maemo_x86_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/maemo \
		make -C /scratchbox/users/admin/home/admin/src/ \
			GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
			qgvdial_maemo_x86_nodeb

############################### harmattan #################################
qgvdial_harmattan_arm:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select HARMATTAN_ARMEL
	/scratchbox/login \
		make -j$(NUMCORES) -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan_armel \
			PRO=./qgvdial/harmattan/harmattan.pro \
			DEBMAKE=build-files/qgvdial/harmattan \
			GITHASH=$(GITHASH) GITREV=$(GITREV) \
			build

qgvdial_harmattan_arm_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/harmattan \
		make -C /scratchbox/users/admin/home/admin/src/ \
			GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
			qgvdial_harmattan_arm

qgvdial_harmattan_arm_nodeb:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select HARMATTAN_ARMEL
	/scratchbox/login \
		make -j$(NUMCORES) -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan_armel \
			PRO=./qgvdial/harmattan/harmattan.pro \
			GITHASH=$(GITHASH) GITREV=$(GITREV) \
			build

qgvdial_harmattan_arm_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/harmattan \
		make -C /scratchbox/users/admin/home/admin/src/ \
			GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
			qgvdial_harmattan_arm_nodeb

qgvdial_harmattan_x86:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select HARMATTAN_X86
	/scratchbox/login \
		make -j$(NUMCORES) -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan_x86 \
			PRO=./qgvdial/harmattan/harmattan.pro \
			DEBMAKE=build-files/qgvdial/harmattan \
			GITHASH=$(GITHASH) GITREV=$(GITREV) \
			build

qgvdial_harmattan_x86_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/harmattan \
		make -C /scratchbox/users/admin/home/admin/src/ \
			GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
			qgvdial_harmattan_x86

qgvdial_harmattan_x86_nodeb:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select HARMATTAN_X86
	/scratchbox/login \
		make -j$(NUMCORES) -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan_x86 \
			PRO=./qgvdial/harmattan/harmattan.pro \
			GITHASH=$(GITHASH) GITREV=$(GITREV) \
			build

qgvdial_harmattan_x86_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/harmattan \
		make -C /scratchbox/users/admin/home/admin/src/ \
			GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
			qgvdial_harmattan_x86_nodeb

qgvdial_harmattan_start_ctr:
	@echo "Run these commands:"
	@echo "  sudo /scratchbox/sbin/sbox_ctl start"
	@echo "  sudo /scratchbox/sbin/sbox_sync"
	@echo "  sb-conf select HARMATTAN_X86"
	@echo "  sb-conf select HARMATTAN_ARMEL"
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/harmattan \
		bash

############################### bb10 #################################
qgvdial_bb10_arm:
	$(MAKE) -j$(NUMCORES) \
		-f ./build-files/qgvdial/Makefile \
		FOR=bb10_arm \
		PRO=./qgvdial/bb10-qt5/bb10.pro \
		'KEYPASS=$(KEYPASS)' \
		build

qgvdial_bb10_arm_srv:
	mkdir -p /tmp/src/build/bb10_arm/qt4srv
	cd /tmp/src/build/bb10_arm/qt4srv ; qmake /tmp/src/qgvdial/bb10-qt5/qt4srv/qt4srv.pro
	make -C /tmp/src/build/bb10_arm/qt4srv all

qgvdial_bb10_arm_ctr:
	# Create the qt4srv first
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/bbndk \
		make -C /tmp/src \
		qgvdial_bb10_arm_srv
	# Then create the rest of the app. This should create the bar package too
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		-v $(GITROOT)/secrets/author.p12:/home/admin/.rim/author.p12 \
		-v $(GITROOT)/secrets/bbidtoken.csk:/home/admin/.rim/bbidtoken.csk \
		accupara/bb10_qt5:arm \
		make -C /tmp/src \
		DEBMAKE=build-files/qgvdial/bb10 \
		'KEYPASS=$(KEYPASS)' \
		qgvdial_bb10_arm

qgvdial_bb10_x86:
	$(MAKE) -j$(NUMCORES) \
		-f ./build-files/qgvdial/Makefile \
		FOR=bb10_x86 \
		PRO=./qgvdial/bb10-qt5/bb10.pro \
		'KEYPASS=$(KEYPASS)' \
		build

qgvdial_bb10_x86_srv:
	mkdir -p /tmp/src/build/bb10_x86/qt4srv
	cd /tmp/src/build/bb10_x86/qt4srv ; qmake /tmp/src/qgvdial/bb10-qt5/qt4srv/qt4srv.pro
	make -C /tmp/src/build/bb10_x86/qt4srv all

qgvdial_bb10_x86_ctr:
	# Create the qt4srv first
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		-e CPUVARDIR=x86 \
		accupara/bbndk \
		make -C /tmp/src \
		CPUVARDIR=x86 \
		qgvdial_bb10_x86_srv
	# Then create the rest of the app. This should create the bar package too
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		-v $(GITROOT)/secrets/author.p12:/home/admin/.rim/author.p12 \
		-v $(GITROOT)/secrets/bbidtoken.csk:/home/admin/.rim/bbidtoken.csk \
		accupara/bb10_qt5:x86 \
		make -C /tmp/src \
		DEBMAKE=build-files/qgvdial/bb10 \
		'KEYPASS=$(KEYPASS)' \
		qgvdial_bb10_x86
