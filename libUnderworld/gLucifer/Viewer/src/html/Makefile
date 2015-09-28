#Requires: Google Closure Compiler
#https://developers.google.com/closure/compiler/
COMP = java -jar compiler/compiler.jar --js=
FLAGS = --js_output_file=
#SCRIPTS = lib/utils.js lib/mouse.js lib/ajax.js lib/html5slider.js lib/sylvester.js lib/webgl.js lib/colour.js lib/colourPicker.js lib/gradient.js draw.js
SCRIPTS = lib.js draw.js

all: template.html

glucifer-all.js: $(SCRIPTS)
	cat $(SCRIPTS) > glucifer-all.js

glucifer.js: glucifer-all.js
	$(COMP)glucifer-all.js $(FLAGS)glucifer.js

template.html: glucifer.js
	sed -e "s/<link.*styles.css\">/<style type=\"text\/css\">\n_STYLESHEET_\n<\/style>/g" index.html > temp.html
	sed -e "s/ src=\"lib.js\">/>\n_SCRIPT_\n/g" temp.html > temp1.html
	sed -e "s/type=\"text\/javascript\" src=\"draw.js\">/id=\"source\" type=\"application\/json\">\n_JSON_DATA_\n/g" temp1.html > temp.html
	sed -e "/_SCRIPT_/r glucifer.js" -e "/SCRIPT/d" temp.html > temp1.html
	sed -e "/_STYLESHEET_/r styles.css" -e "/_STYLESHEET_/d" temp1.html > template.html
	rm temp.html
	rm temp1.html
	rm glucifer-all.js

.PHONY : clean
clean:
	-rm template.html
	-rm glucifer.js
