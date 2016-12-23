GITROOT  := $(shell git rev-parse --show-toplevel)

build:
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; rm -rf debian temp ../qgvdial*tar*
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; USER=admin dh_make --yes --createorig --single -e 'yuvraaj@gmail.com' -c lgpl
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; mv debian temp ; mkdir debian ; cd temp ; mv copyright compat ../debian ; cd .. ; rm -rf temp
	cp changelog control install rules $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/debian
	-cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; dpkg-buildpackage -rfakeroot -nc -uc -us

full32_in_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_i386 \
		make -j4 -C /tmp/src/ -f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		DEBMAKE=build-files/qgvdial/ubuntu \
		build

full64_in_ctr:
	docker run \
		--rm -it \
		-v $(GITROOT):/tmp/src \
		accupara/qgvdial_qt5_amd64 \
		make -j4 -C /tmp/src/ -f ./build-files/qgvdial/Makefile \
		FOR=ubuntu_x86_64 \
		PRO=./qgvdial/qt-not-qml/desktop_linux.pro \
		DEBMAKE=build-files/qgvdial/ubuntu \
		build
