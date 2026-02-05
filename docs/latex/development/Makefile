include ../../../Makefile.vars

all: development.pdf

development.pdf: *.tex
	latexmk -lualatex development.tex

# tarprep: ${DOCTARDIR}/development.pdf

${DOCTARDIR}/development.pdf: development.pdf
	${INSTALL} -c -m 644 $< $@

clean::
	rm -f *.log *.aux development.fdb_latexmk development.fls development.lof \
	  development.lot development.out development.toc

distclean:: clean

