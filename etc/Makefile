include ../Makefile.vars

TARTARGS = ${TARDIR}/etc/mclogd.cfg.example

all::
	echo "nothing to be done for 'all' target."

tarprep: ${TARTARGS} tarprep-${OSNAME}

tarprep-freebsd::
	${MAKE} -C freebsd tarprep

tarprep-linux::
	${MAKE} -C linux/systemd/system tarprep

tarprep-darwin::
	${MAKE} -C macos tarprep

	echo "No startup scripts for darwin"

${TARDIR}/etc/mclogd.cfg.example: mclogd.cfg.example
	../install-sh -c -m 444 $< $@

clean:: tarprep-clean

tarprep-clean:: clean-tarprep-${OSNAME}
	rm -f ${TARTARGS}

clean-tarprep-freebsd::
	${MAKE} -C freebsd/rc.d clean-tarprep

clean-tarprep-linux::
	${MAKE} -C linux/systemd/system clean-tarprep

clean-tarprep-darwin::
	${MAKE} -C macos clean-tarprep

distclean::
	echo "Nothing to be done for distclean"
