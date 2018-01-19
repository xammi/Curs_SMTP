#################################################
# This makefile is for generating report only
# Depends on latex and tex cyrillic extension

# To send sources: scp -P 9999 -r report maxim@localhost:/home/maxim
# To get result: scp -P 9999 maxim@localhost:/home/maxim/report/report.pdf report/.
#################################################

REPORT = report.tex

first: report

report:
	pdflatex $(REPORT)

clean:
	rm -f $(REPORT)
	rm -f *.out *.aux *.log *.toc
