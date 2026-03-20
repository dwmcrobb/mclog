include ../../Makefile.vars

PKGTARGETS = ${TARDIR}/man/man1/mclog.1 \
             ${TARDIR}/man/man8/mclogd.8 \
             ${TARDIR}/man/man5/mclogd.cfg.5

all: mclog.html mclogd.html mclogd.cfg.html

tarprep:: ${PKGTARGETS}

${TARDIR}/man/man1/mclog.1: mclog.1
	../../install-sh -c -m 644 $< $@

${TARDIR}/man/man8/mclogd.8: mclogd.8
	../../install-sh -c -m 644 $< $@

${TARDIR}/man/man8/mclogd.cfg.5: mclogd.cfg.5
	../../install-sh -c -m 644 $< $@

mclog.html: mclog.1 mclog.css
	mandoc -T html -O style=./mclog.css $< > $@

mclogd.html: mclogd.8 mclog.css
	mandoc -T html -O style=./mclog.css $< > $@

mclogd.cfg.html: mclogd.cfg.5 mclog.css
	mandoc -T html -O style=./mclog.css $< > $@

clean::
	rm -f mclog.html mclogd.html mclogd.cfg.html

distclean:: clean tarprep-clean

tarprep-clean::
	rm -f ${PKGTARGETS}
