GITROOT  := $(shell git rev-parse --show-toplevel)

nothing:
	@echo "The default target will do nothing. Please use one of the defined targets"

############################### x86_64 #################################
qgvdial_ubuntu_x86_64:
	$(MAKE) -j4 \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86_64 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		DEBMAKE=build-files/qgvdial/ubuntu \
		build

qgvdial_ubuntu_x86_64_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_amd64 \
		make -C /tmp/src \
		qgvdial_ubuntu_x86_64

qgvdial_ubuntu_x86_64_nodeb:
	$(MAKE) -j4 \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86_64 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		build

qgvdial_ubuntu_x86_64_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_amd64 \
		make -C /tmp/src \
		qgvdial_ubuntu_x86_64_nodeb

############################### x86 #################################
qgvdial_ubuntu_x86:
	$(MAKE) -j4 \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		DEBMAKE=build-files/qgvdial/ubuntu \
		build

qgvdial_ubuntu_x86_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_i386 \
		make -C /tmp/src \
		qgvdial_ubuntu_x86

qgvdial_ubuntu_x86_nodeb:
	$(MAKE) -j4 \
		-f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		build

qgvdial_ubuntu_x86_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_i386 \
		make -C /tmp/src \
		qgvdial_ubuntu_x86_nodeb

############################### maemo #################################
qgvdial_maemo:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select FREMANTLE_ARMEL
	/scratchbox/login \
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo \
			PRO=./qgvdial/maemo/maemo.pro \
			DEBMAKE=build-files/qgvdial/maemo \
			build

qgvdial_maemo_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/maemo \
		make -C /scratchbox/users/admin/home/admin/src/ qgvdial_maemo

qgvdial_maemo_nodeb:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select FREMANTLE_ARMEL
	/scratchbox/login \
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=maemo \
			PRO=./qgvdial/maemo/maemo.pro \
			build

qgvdial_maemo_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/maemo \
		make -C /scratchbox/users/admin/home/admin/src/ qgvdial_maemo_nodeb

############################### harmattan #################################
qgvdial_harmattan:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select FREMANTLE_ARMEL
	/scratchbox/login \
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan \
			PRO=./qgvdial/harmattan/harmattan.pro \
			DEBMAKE=build-files/qgvdial/harmattan \
			build

qgvdial_harmattan_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/harmattan \
		make -C /scratchbox/users/admin/home/admin/src/ qgvdial_harmattan

qgvdial_harmattan_nodeb:
	sudo /scratchbox/sbin/sbox_ctl start
	sudo /scratchbox/sbin/sbox_sync
	sb-conf select FREMANTLE_ARMEL
	/scratchbox/login \
		make -j4 -C ~/src -f ./build-files/qgvdial/Makefile \
			GITROOT=~/src \
			FOR=harmattan \
			PRO=./qgvdial/harmattan/harmattan.pro \
			build

qgvdial_harmattan_nodeb_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/harmattan \
		make -C /scratchbox/users/admin/home/admin/src/ qgvdial_harmattan_nodeb
