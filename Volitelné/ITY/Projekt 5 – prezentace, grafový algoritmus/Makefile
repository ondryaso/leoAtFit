JNAME=proj5
INPUT=${JNAME}.tex

${JNAME}.pdf: ${INPUT}
	pdflatex -jobname ${JNAME} $<
	pdflatex -jobname ${JNAME} $<

.PHONY: clean
clean:
	rm -f ${JNAME}.aux ${JNAME}.out ${JNAME}.log ${JNAME}.pdf
