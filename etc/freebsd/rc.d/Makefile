include ../../../Makefile.vars

TARTARGETS  = ${STAGING_DIR}/usr/local/etc/rc.d/mclogd

all::
	echo "Nothing to be done for 'all'."

tarprep:: ${TARTARGETS}

${STAGING_DIR}/usr/local/etc/rc.d/mclogd: mclogd
	../../../install-sh -c -m 755 $< $@

clean:: clean-tarprep

clean-tarprep::
	rm -f ${TARTARGETS}

