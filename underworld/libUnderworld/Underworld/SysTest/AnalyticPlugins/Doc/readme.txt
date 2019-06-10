To view the documentation associated with these analytic solutions you must first build the documentation.
To build it you must have a working copy of 'latex' and 'dvipdf', then type:
	make pdf

Now a ready made .pdf document should be ready in your directory. (*)

For those without dvipdf, (what are you thinking), type:
	make doc
and view output with:
	xdvi all_solsn.dvi


'make clean' will do also work to clean up directory

Update
======

\begin{commandments}
  0) thou shalt use dvips to print thy stone tablets
\end{commandments}

pdflatex is an acceptable third millennium alternative for the stone-tablet-less office.
