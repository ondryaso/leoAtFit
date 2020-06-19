INPUT=proj1.tex
OUTPUT=proj1.pdf

${OUTPUT}: proj_ity.ps
	ps2pdf -sPAPERSIZE=a4 $< $@

proj_ity.ps: proj_ity.dvi
	dvips -t a4 -o $@ $<

proj_ity.dvi: ${INPUT}
	latex -jobname proj_ity $<
	latex -jobname proj_ity $<

.PHONY: clean
clean:
	rm -f proj_ity.dvi proj_ity.aux proj_ity.ps proj_ity.out proj_ity.log ${OUTPUT}
