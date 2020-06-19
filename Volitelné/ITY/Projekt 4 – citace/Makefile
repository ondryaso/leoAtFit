JNAME=proj4
INPUT=${JNAME}.tex

${JNAME}.pdf: ${INPUT}
	pdflatex -jobname ${JNAME} $<
	biber ${JNAME}
	pdflatex -jobname ${JNAME} $<
	pdflatex -jobname ${JNAME} $<

.PHONY: clean
clean:
	rm -f ${JNAME}.run.xml ${JNAME}.bbl ${JNAME}.bcf ${JNAME}.blg ${JNAME}.aux ${JNAME}.out ${JNAME}.log ${JNAME}.pdf
