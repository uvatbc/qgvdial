build:
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; rm -rf debian temp ../qgvdial*tar*
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; USER=admin dh_make --yes --createorig --single -e 'yuvraaj@gmail.com' -c lgpl
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; mv debian temp ; mkdir debian ; cd temp ; mv copyright compat ../debian ; cd .. ; rm -rf temp
	cp control install rules $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/debian
	envsubst <changelog > $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/debian/changelog
	-cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; dpkg-buildpackage -rfakeroot -nc -uc -us
