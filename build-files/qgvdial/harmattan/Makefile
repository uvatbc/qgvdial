GITROOT  ?= $(shell git rev-parse --show-toplevel)

build:
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; rm -rf debian temp ../qgvdial*tar*
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; USER=admin dh_make --yes --createorig --single -e 'yuvraaj@gmail.com' -c lgpl
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; mv debian temp ; mkdir debian ; cd temp ; mv copyright compat ../debian ; cd .. ; rm -rf temp
	cd $(GITROOT)/qgvdial/maemo/qtc_packaging/debian_fremantle ; cp control install rules $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/debian
	envsubst <changelog > $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/debian/changelog
	-cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; dpkg-buildpackage -rfakeroot -nc -uc -us

it:
	docker run \
		--rm -it \
		-v $(GITROOT):/scratchbox/users/admin/home/admin/src \
		--privileged \
		accupara/harmattan \
		/bin/bash
