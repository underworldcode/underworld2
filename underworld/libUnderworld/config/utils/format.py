def box(text, width, indent_size=0):

    first = True
    new_text = ""
    lines = text.splitlines()
    for line in lines:

        if not first:
            new_text += "\n"
        else:
            first = False

        pos = 0
        words = line.split()
        for word in words:

            if pos:
                if pos + len(word) + 1 > width:
                    word = indent_size*" " + word
                    new_text += "\n" + word
                    pos = len(word)

                else:
                    new_text += " " + word
                    pos += len(word) + 1

            else:
                word = indent_size*" " + word
                new_text += word
                pos += len(word)

    if text[-1] == "\n":
        new_text += "\n"

    return new_text
