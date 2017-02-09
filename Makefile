GITROOT ?= $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
GITHASH ?= $(shell git log -1 --format=%H)
GITREV  ?= $(shell git rev-list --count $(GITHASH))

nothing:
	@echo "The default target will do nothing. Please use one of the defined targets"

############################### x86_64 #################################
qgvdial_ubuntu_x86_64:
	$(MAKE) -j4 \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86_64 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		DEBMAKE=build-files/qgvdial/ubuntu \
		GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
		build

qgvdial_ubuntu_x86_64_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_amd64 \
		make -C /tmp/src \
		GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
		qgvdial_ubuntu_x86_64

qgvdial_ubuntu_x86_64_nodeb:
	$(MAKE) -j4 \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86_64 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
		build

qgvdial_ubuntu_x86_64_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_amd64 \
		make -C /tmp/src \
		GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
		qgvdial_ubuntu_x86_64_nodeb

############################### x86 #################################
qgvdial_ubuntu_x86:
	$(MAKE) -j4 \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		DEBMAKE=build-files/qgvdial/ubuntu \
		GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
		build

qgvdial_ubuntu_x86_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_i386 \
		make -C /tmp/src \
		GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
		qgvdial_ubuntu_x86

qgvdial_ubuntu_x86_nodeb:
	$(MAKE) -j4 \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
		build

qgvdial_ubuntu_x86_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_i386 \
		make -C /tmp/src \
		GITROOT=$(GITROOT) GITHASH=$(GITHASH) GITREV=$(GITREV) \
		qgvdial_ubuntu_x86_nodeb

############################### maemo #################################
qgvdial_maemo_armel:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select FREMANTLE_ARMEL
	/scratchbox/login \
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo \
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
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo \
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
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo \
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
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo \
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
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan \
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
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan \
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
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan \
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
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan \
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
