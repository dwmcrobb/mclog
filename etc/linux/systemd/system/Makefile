include ../../../../Makefile.vars

TARTARGETS  = ${STAGING_DIR}/etc/systemd/system/mclogd.service

all::
	echo "Nothing to be done for 'all'."

tarprep:: ${TARTARGETS}

${STAGING_DIR}/etc/systemd/system/mclogd.service: mclogd.service
	../../../../install-sh -c -m 644 $< $@

clean:: clean-tarprep

clean-tarprep::
	rm -f ${TARTARGETS}

