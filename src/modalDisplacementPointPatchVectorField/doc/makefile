all:
	pdflatex main
	bibtex main
	pdflatex main
	pdflatex main


pack:
	pack.sh

.PHONY: clean
clean:
	rm -rf *.aux *.blg *.bbl *.log
	rm~all
