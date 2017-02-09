GITROOT  ?= $(shell git rev-parse --show-toplevel)

build:
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; rm -rf debian temp ../qgvdial*tar*
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; export USER=admin ; echo y | dh_make -n --createorig --single -e 'yuvraaj@gmail.com' -c lgpl2
	cd $(GITROOT)/qgvdial/maemo/qtc_packaging/debian_fremantle ; cp compat control copyright README rules $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/debian
	#envsubst <changelog > $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/debian/changelog
	-cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; dpkg-buildpackage -rfakeroot

it:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/maemo \
		/bin/bash
