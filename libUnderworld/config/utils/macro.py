import re
import conv


# def is_macro(word):
#     if word[:2] == "${":
#         if word.find("}"):
#             return True
#     elif word[0] == "$":
#         return True
#     else:
#         return False




# def expand_macro(macro, env):
#     # Strip away the macro indicators.
#     if macro[:2] == '${':
#         i = macro.find("}")
#         macro = macro[2:i] + macro[i + 1:]
#     elif macro[0] == '$':
#         macro = macro[1:]

#     # Check if we have an argument list for a callable.
#     if macro[-1] == ")":
#         begin = macro.find("(")

#         # Process arguments.
#         i = begin
#         while i != -1:
#             rem = macro[i + 1:]
#             if is_macro(rem):
#             i = cur.find(",")
#             args.append(macro[i + 1:])

#         macro = macro[:begin]
#     else:
#         args = []

#     if macro not in env:
#         res = expand_dict(macro, env)
#         if not res:
#             return ''
#     else:
#         res = env[macro]

#     # If the expansion is callable, call it with the environment
#     # as the argument.
#     if callable(res):

#         # Expand the argument list.
#         for i in range(len(args)):
#             args[i] = subst(args[i], env, string=False)

#         res = res(env, *args)
#         # TODO: What does the callable return? If it can return another
#         # callable we'll have to repeat the process.

#     return res

# def handle_new_line(new_line):
#     if isinstance(new_line, str):
#         new_words = [nw.strip() for nw in new_line.split()]
#     elif isinstance(new_line, list):
#         new_words = [handle_new_line(i) for i in new_line]
#     else:
#         raise "Can only support environment entries of either string or list."
#     return new_words

# def subst(line, env, string=True):
#     inp = [w.strip() for w in line.split()]
#     out = []
#     while len(inp):
#         w = inp.pop(0)
#         if is_macro(w):
#             new_line = expand_macro(w, env)
#             new_words = handle_new_line(new_line)
#             inp = conv.flatten(new_words) + inp
#         else:
#             out.append(w)
#     if string or len(out) <= 1:
#         return " ".join(out)
#     else:
#         return out







reduced_macro_re = r"\$([a-zA-Z0-9_\.]*)"
explicit_macro_re = "\{\$([a-zA-Z0-9\.]*)\}"
macro_re = r"(?:" + explicit_macro_re + r")|(?:" + reduced_macro_re + ")"
func_re = reduced_macro_re + r"\("

func_or_macro_re = r"(?:" + func_re + ")|(?:" + macro_re + ")"
func_continue_re = r"(?:" + func_or_macro_re + ")|(\))"

arg_re = "(?:" + func_re + ")|(?:,)|(?:\))"

func_or_macro_prog = re.compile(func_or_macro_re)
func_continue_prog = re.compile(func_continue_re)
arg_prog = re.compile(arg_re)


def expand_dict(macro, env):
    subs = macro.split(".")
    for s in subs:
        if s not in env:
            return ''
        env = env[s]
    return env


def parse_args(line, env, pos):
    cur_arg = ""
    args = []
    m = arg_prog.search(line, pos)
    while m:
        span = m.span()
        if m.group(1) is not None:
            macro = m.group(1)
        else:
            macro = None

        # Add unmatched text to the current argument and update position.
        cur_arg += line[pos:span[0]]
        pos = span[1]

        # If we found another function...
        if macro is not None:

            # Pass through unmatched text.
            func = env.get(macro, None)
            if func:
                if not callable(func):
                    raise "Non-callable used as function."
                res = parse_args(line, env, pos)
                res2 = subst(func(env, *res[0]), env, string=False)
                if not cur_arg:
                    cur_arg = res2
                else:
                    cur_arg += res2
                pos = res[1]

        else:
            args.append(subst(cur_arg, env, string=False))
            cur_arg = ""

            # Check if we've hit the end of the argument list.
            if line[pos - 1] == ")":
                break

        m = arg_prog.search(line, pos)

    return (args, pos)


def subst(line, env, pos=0, string=True):
    # If we were given a list, substitute each element.
    if isinstance(line, list):
        res = [subst(i, env, string=string) for i in line]
        if string:
            return " ".join(res)
        else:
            return res

    # If we don't have a string, return unchanged.
    elif not isinstance(line, str):
        if string:
            return str(line)
        else:
            return line

    # Substitute string.
    out = ""
    m = func_or_macro_prog.search(line, pos)
    while m:
        span = m.span()
        if m.group(3) is not None:
            macro = m.group(3)
        elif m.group(2) is not None:
            macro = m.group(2)
        else:
            macro = m.group(1)

        # Pass all the text we missed into the output.
        out += line[pos:span[0]]

        # Update the position.
        pos = span[1]

        # If we matched a function, handle the contents.
        if line[span[1] - 1] == "(":
            func = env.get(macro, None)
            if not callable(func):
                raise "Non-callable used as function."
            res = parse_args(line, env, pos)
            pos = res[1]
            res = subst(func(env, *res[0]), env, string=string)

        else:
            if macro not in env:
                # Could be a dictionary...
                macro = expand_dict(macro, env)
            else:
                # Substitute.
                macro = env.get(macro, "")

            # Append to output after further substitution.
            res = subst(macro, env, string=string)

        # If we don't have a string, just return.
        if not isinstance(res, str):
            if span[0] != 0 or pos != len(line):
                raise "Cannot combine non-strings..."
            return res
        else:
            out += res

        # Keep searching from the end of the match.
        m = func_or_macro_prog.search(line, pos)

    return out + line[pos:]
