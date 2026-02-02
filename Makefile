load $(shell pkg-config --variable=libdir dwmgmk)/dwm_gmk.so(dwm_gmk_setup)

$(dwm_include Makefile.vars)
$(dwm_include apps/Makefile)

tarprep: otherTarpreps

otherTarpreps::
	${MAKE} -C etc tarprep
	${MAKE} -C packaging tarprep
#	${MAKE} -C docs tarprep

package: ${OSNAME}-pkg

freebsd-pkg: tarprep
	mkfbsdmnfst -r packaging/fbsd_manifest -s staging classes/tests > staging/+MANIFEST
	pkg create -o . -r staging -m staging

darwin-pkg: tarprep
	pkgbuild --root staging --identifier net.mcplex.mclog --version ${VERSION} --scripts etc/macos/scripts mclog-${VERSION}.pkg

linux-pkg: tarprep
	if [ ! -d staging/DEBIAN ]; then mkdir staging/DEBIAN; fi
	mkdebcontrol -r packaging/debcontrol -s staging/usr/local classes/tests > staging/DEBIAN/control
	dpkg-deb -b --root-owner-group staging
	dpkg-name -o staging.deb

distclean:: otherDistclean

otherDistclean::
	@${MAKE} -s -C packaging distclean
	@rm -Rf autom4te.cache staging
	@rm -f config.log config.status Makefile.vars
	@rm -f mclog_*.deb mclog-*.pkg

