build:
	cp ../../../qgvdial/bb10-qt5/bar-descriptor-deploy.xml $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION)/bar-descriptor.xml
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; if [ `file qgvdial | grep -c ARM` != 0 ] ; then ntoarmv7-strip qgvdial ; else strip qgvdial ; fi
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; if [ `file ../qt4srv/qt4srv | grep -c ARM` != 0 ] ; then ntoarmv7-strip ../qt4srv/qt4srv ; else strip ../qt4srv/qt4srv ; fi
	cd $(BUILDDIR)/$(FOR)/qgvdial-$(VERSION) ; blackberry-nativepackager -package qgvdial_$(VERSION).bar bar-descriptor.xml