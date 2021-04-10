#
# File: options.py
#
# An option is specified as:
#   <flag>[separator][value]([<delimiter><value>])*
#

import re
from . import conv, format

#
# Class: OptionVariant
#

class OptionVariant(object):
    """OptionVariant represents the different ways a single option
    may be specified."""

    def __init__(self, flags, type="string", **kw):
        """flags: Either a string or list of the exact flags that
        can represent this option variant.
        type: A string representing the kind of option. One of "string",
        "enum" or "bool"."""

        # Convert the flags to a list.
        self.flags = conv.to_list(flags)

        # Extract parameters from the keywords.
        self.seps = conv.to_list(kw.get("seps", ["=", " "]))
        self.pref_sep = sep and kw.get("pref_sep", sep[0]) or None
        self.delims = conv.to_list(kw.get("delims", ""))
        self.pref_delim = delim and kw.get("pref_delim", delim[0]) or None

        # Error on duplicate flags.
        for i in range(len(self.flags) - 1):
            if self.flags[i] in self.flags[i + 1:]:
                raise "Duplicate flag given to OptionVariant."

        # Error on duplicate separators.
        for i in range(len(self.sep) - 1):
            if self.sep[i] in self.sep[i + 1:]:
                raise "Duplicate separators given to OptionVariant."

        # Error on separators that are not single characters.
        for s in self.seps:
            if len(s) != 1:
                raise "Separator of length != 1."

        # Error on duplicate delimiters.
        for i in range(len(self.delims) - 1):
            if self.delims[i] in self.delims[i + 1:]:
                raise "Duplicate delimiter given to OptionVariant."

        # Error on delimiters that are not single characters or
        # delimiters that are whitespaces.
        for d in self.delims:
            if len(d) != 1:
                raise "Delimeter of length != 1."
            if d == " ":
                raise "Whitespace delimeter."

        # Sort the separators and delimiters so that we don't mistake shorter
        # ones for longer ones.
        self.seps.sort(lambda x,y: len(y)-len(x))
        self.delims.sort(lambda x,y: len(y)-len(x))

        #
        # Handle the different kinds of option types. We need to set
        # the conversion method "to_str", set the parsing method
        # "parse_val" and setup any type specific details.
        #

        if self.type == "bool":

            self.to_str = self._bool_to_str
            self.parse_val = self._parse_bool

            # Boolean options accept an optional parameter defining
            # what indicates a positive and negative value. It should be
            # specified as a tuple of two lists, the first being the
            # positive values the second the negative.
            bf = kw.get("bool_values", (["", "yes", "1", "t"], ["no", "0", "f"]))

            # Boolean's also accept additional parameters for specifying
            # which positive and negative values are preferential. These
            # will influence conversion of option lists to strings. By default
            # the first values in the "bool_values" list are considered
            # preferential.
            self._pref_true_flag = kw.get("pref_true_value", conv.to_list(bf[0])[0])
            self._pref_false_flag = kw.get("pref_false_value", conv.to_list(bf[1])[0])

            # Setup the values. We sort the list by the length of the values
            # in descending order so that when we parse option strings we don't
            # mistake shorter options for longer ones.
            self.bool_values = (zip(conv.to_list(bf[0]), [True for i in range(len(bf[0]))]) +
                               zip(conv.to_list(bf[1]), [False for i in range(len(bf[1]))]))
            self.bool_values.sort(lambda x,y: len(y[0])-len(x[0]))

        elif self.type == "enum":

            self.to_str = self._enum_to_str
            self.parse_val = self._parse_enum

            # An additional option for enumerations is "enum", which defines a
            # a mapping from the string values expected in the option strings to
            # an internal string representation. It accepts either a dictionary
            # or a string.  In the case of a list each entry maps to itself.
            self.enum = kw.get("enum", {})
            if isinstance(self.enum, list):
                self.enum = dict(zip(self.enum, self.enum))

        else:

            self.to_str = self._str_to_str
            self.parse_val = self._parse_str

    def _bool_to_str(self, val):
        """Convert a true/false value to a an option string."""

        f = val and self.pref_true_flag or self.pref_false_flag
        if f:
            s = "%s%s%s" % (self.flags[0], self.pref_sep, f)
        else:
            s = "%s" % self.flags[0]
        return s.strip()

    def _parse_bool(self, val_str):
        """Convert a string to a boolean value."""

        for f, v in self.bool_values:
            if f == "":
                return (v, False)
            if f == val_str:
                return (v, True)
        return None

    def _enum_to_str(self, val):
        val = conv.to_list(val)
        s = []
        for v in val:
            e = self.enum.get(v, None)
            if e is None:
                if v not in self.enum.values():
                    raise "Invalid enumerated option."
                e = v
            s.append(e)
        return self._str_to_str(self._pref_delim.join(s))

    def _parse_enum(self, val):
        if not val:
            return None
        if val not in self.enum:
            raise "Invalid enumerated option."
        return (val, True)

    def _str_to_str(self, val):
        val = conv.to_list(val)
        return "%s%s%s" % (self.flags[0], self._pref_sep, self._pref_delim.join(val))

    def _parse_str(self, val):
        if not val:
            return None
        return (val, True)

    def match_sep(self, val_str, b):
        for s in self.sep:
            ls = len(s)
            if ls <= len(val_str) - b and s == val_str[b:b + ls]:
                return b + ls
        return None

    def __repr__(self):
        return "/".join(self.flags)

#
# Class: Option
#

class Option(object):
    """A container for a set of OptionVariant instances. Each option
    has to have a single type."""

    def __init__(self, name, *args, **kw):
        self.name = name

        # If we were given a 'variants' keyword then take that as
        # one or a list of OptionVariant instances.
        vars = kw.get("variants", None)
        if vars is None:
            # Otherwise all the parameters we were given are to create
            # a new OptionVariant.
            vars = OptionVariant(*args, **kw)
        self.variants = conv.to_list(vars)

        # We take the type of the first variant to be the type of
        # this option.
        self.type = self.variants[0].type

        # This mapping is for moving from a flag string to the variant
        # that represents it.
        self._flg_to_var = {}

        for v in self.variants:

            # Make sure we don't have multiple types in the variants.
            if self.type != v.type:
                raise "Inconsistent OptionVariant types."

            for f in v.flags:

                # Make sure we don't have multiple variants trying to
                # represent the same flag.
                if f in self._flg_to_var:
                    raise "Duplicate option flags in Option."

                # Inser the flag/variant into the mapping.
                self._flg_to_var[f] = v

        self.help = kw.get("help", None)
        self.default = kw.get("default", None)

    def get_variant(self, flg):
        return self._flg_to_var[flg]

    def make_help_string(self):
        v = self.variants[0]
        if len(v.flags) > 1:
            help = "%s" % "|".join(v.flags)
        else:
            help = str(v.flags[0])
        if v.type == "string":
            help += v._pref_sep + "<string>"
        elif v.type == "enum":
            help += v._pref_sep + "<%s>" % "|".join(v.enum.values())
        elif v.type == "bool":
            if v._pref_true_flag is "":
                help = "[" + help + "]"
            else:
                help += v._pref_sep + "<" + v._pref_true_flag + \
                    "|" + v._pref_false_flag + ">"
        if self.default is not None:
            help += "\n\tdefault: %s" % str(self.default)
        if self.help:
            txt = format.box(self.help, 60)
            help += "\n\t" + "\n\t".join(txt.split("\n"))
        return help

    def __repr__(self):
        s = []
        for v in self.variants:
            s.append(str(v))
        return "/".join(s)

#
# Class: OptionSet
#

class OptionSet(object):

    Option = Option

    def __init__(self):
        self.options = {}
        self._order = []
        self._flg_to_opt = {}
        self._expr = None

    def __contains__(self, opt_name):
        return opt_name in self.options

    def add_option(self, opts, *args):
        opts = conv.to_list(opts)
        opts += args
        for o in opts:
            if o.name in self.options:
                raise RuntimeError("Duplicate option name %s."%repr(o.name))
            self.options[o.name] = o
            self._order.append(o.name)
            for f in o._flg_to_var.iterkeys():
                if f in self._flg_to_opt:
                    raise "Duplicate option flag in OptionSet."
                self._flg_to_opt[f] = o
        self._expr = None

    def remove_option(self, name):
        for f in self.options[name]._flg_to_var.iterkeys():
            del self._flg_to_opt[f]
        del self.options[name]
        del self._order[self._order.index(name)]
        self._expr = None

    def pop(self, name):
        opt = self.options[name]
        self.remove_option(name)
        return opt

    def has_option(self, name):
        return name in self.options

    def get_flags(self):
        return self._flg_to_opt.keys()
    flags = property(get_flags, None)

    def merge(self, opts):
        for name in opts._order:
            if name not in self.options:
                self.add_option(opts.options[name])

    def merge_option(self, opt):
        if opt.name not in self.options:
            self.add_option(opt)
        else:
            # TODO: won't matter for now, but need to do this.
            pass

    def parse_option_string(self, opt_str):
        return self.parse_option_list(opt_str.split())

    def parse_option_list(self, words):
        found_opts = set()
        if not self._expr:
            self._make_expr()
        opt_l = []
        while len(words):
            w = words.pop(0)
            m = self._expr.match(w)
            if m is None or not m.groups():
                opt_l.append(w)
                continue
            e = m.end(m.lastindex)
            flg = w[:e]
            opt = self._flg_to_opt[flg]
            var = opt.get_variant(flg)
            if e < len(w):
                i = var.match_sep(w, e)
                if i is not None:
                    next = w[i:]
                    res = var.parse_val(next)
                    if res is None or not res[1]:
                        opt_l.append(w)
                        continue
                    opt_l.append((opt.name, res[0]))
                    found_opts.add(opt.name)
                else:
                    opt_l.append(w)
                    continue
            elif " " in var.sep:
                if not len(words):
                    next = ""
                else:
                    next = words.pop(0)
                res = var.parse_val(next)
                if res is None:
                    opt_l.append(w)
                    if next:
                        words.insert(0, next)
                    continue
                opt_l.append((opt.name, res[0]))
                found_opts.add(opt.name)
                if not res[1] and next:
                    words.insert(0, next)
            else:
                opt_l.append(w)
                continue
        opt_l += self.make_defaults(found_opts)
        return opt_l

    def make_defaults(self, found_opts):
        defs = []
        for name, opt in self.options.iteritems():
            if opt.default is None or name in found_opts:
                continue
            defs.append((opt.name, opt.variants[0].parse_val(opt.default)[0]))
        return defs

    def make_option_string(self, args):
        s = []
        for a in args:
            if isinstance(a, tuple) and len(a) == 2:
                s.append(self.options[a[0]].variants[0].to_str(a[1]))
            elif isinstance(a, str):
                s.append(a)
            else:
                s.append(str(a))
        return " ".join(s)

    def gather(self, args):
        d = {}
        for a in args:
            if isinstance(a, tuple) and len(a) == 2:
                if a[0] in d:
                    if not isinstance(d[a[0]], list):
                        d[a[0]] = conv.to_list(d[a[0]])
                    d[a[0]].append(a[1])
                else:
                    d[a[0]] = a[1]
        return d

    def make_help_string(self):
        help = []
        for o in self._order:
            help.append(self.options[o].make_help_string())
        return "\n".join(help) + "\n"

    def _make_expr(self):
        flags = []
        for f, o in self._flg_to_opt.iteritems():
            flags.append((f, o))
        flags.sort(lambda x,y: len(y[0])-len(x[0]))
        l = []
        for f in flags:
            l.append("(%s)" % f[0])
        self._expr = re.compile("|".join(l))

    def __repr__(self):
        return str(self.options)
