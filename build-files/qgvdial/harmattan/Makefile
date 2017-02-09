GITROOT ?= $(shell git rev-parse --show-toplevel)

build:
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; rm -rf debian temp ../qgvdial*tar*
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; export USER=admin ; echo y | dh_make -n --createorig --single -e 'yuvraaj@gmail.com' -c lgpl2
	cd $(GITROOT)/qgvdial/harmattan/qtc_packaging/debian_harmattan ; cp compat control copyright qgvdial.aegis README rules $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/debian
	#envsubst <changelog > $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/debian/changelog
	-cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; dpkg-buildpackage -rfakeroot -nc -b -uc -us

it:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/harmattan \
		/bin/bash
