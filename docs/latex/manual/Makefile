include ../../../Makefile.vars

all: mclog_manual.pdf

mclog_manual.pdf: *.tex
	latexmk -lualatex mclog_manual.tex

tarprep: ${DOCTARDIR}/mclog_manual.pdf

${DOCTARDIR}/mclog_manual.pdf: mclog_manual.pdf
	${INSTALL} -c -m 644 $< $@

clean::
	rm -f *.log *.aux mclog_manual.fdb_latexmk mclog_manual.fls mclog_manual.lof \
	  mclog_manual.lot mclog_manual.out mclog_manual.toc

distclean:: clean

