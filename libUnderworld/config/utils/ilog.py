import os
from . import format


class IndentLogger(object):

    def __init__(self, level=0):
        self.indentation = 0
        self.level = level


    def set_filename(self, filename):
        self.filename = os.path.abspath(filename)
        try:
            os.remove(filename)
        except:
            pass


    def indent(self, indent=1):
        if self.indentation + indent < 0:
            raise "Too many unindentations."
        self.indentation += indent


    def unindent(self, indent=1):
        self.indent(-indent)


    def __call__(self, msg, level=50, pre_indent=0, post_indent=0, width=100, sub_indent=True):
        if level < self.level:
            return

        f = open(self.filename, "a")

        width = width - 2*self.indentation
        if width <= 0:
            raise "Indented to far for width."

        self.indent(pre_indent)
        lines = msg.split("\n")
        for line in lines:
            if not line:
                continue
            more_lines = format.box(line, width).split("\n")
            f.write(self.indentation*"  " + more_lines[0].strip() + "\n")
            if sub_indent:
                self.indentation += 2
            for l in more_lines[1:]:
                if not l:
                    continue
                f.write(self.indentation*"  " + l.strip() + "\n")
            if sub_indent:
                self.indentation -= 2
        self.indent(post_indent)

        f.close()

log = IndentLogger()
