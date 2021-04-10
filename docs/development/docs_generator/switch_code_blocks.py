from sys import argv
import re

for arg in argv[1:]:
    with open(arg,'r') as fileg:
        text = fileg.read()
        pattern = r"(.. code:: ipython3)((\n +.*|\s)+)"
        # pattern = r"(.. code:: ipython3\s+$)((\n +.*|\s)+)"  # not working

        p = re.compile(pattern)

        def escape_open_bracket(matchobj):
            blockstr = ".. raw:: html\n\n    <pre data-executable> "
            # note we need to escape < with &lt; else it interferes with html tags
            blockstr += matchobj.group(2).replace('<','&lt;')
            blockstr += "    </pre>\n\n"
            return blockstr

        result = p.sub(escape_open_bracket, text)

    with open(arg,'w') as fileg:
        fileg.write(result)

    print("Switched code blocks: {}".format(arg))
