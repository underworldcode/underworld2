from __future__ import print_function,  absolute_import
from pint import UnitRegistry
from itertools import chain
from collections import OrderedDict
import functools
from pint.compat import zip_longest
from pint.errors import DimensionalityError

try:              # Python 2
    str_base = basestring
    items = 'iteritems'
except NameError: # Python 3
    str_base = str, bytes, bytearray
    items = 'items'

_RaiseKeyError = object()  # singleton for no-default behavior

u = UnitRegistry = UnitRegistry()


def ensure_lower(maybe_str):
    """dict keys can be any hashable object - only call lower if str"""
    return maybe_str.lower() if isinstance(maybe_str, str_base) else maybe_str


def ensure_to_base_units(val):
    return val.to_base_units()


class TransformedDict(dict):  # dicts take a mapping or iterable as their optional first argument
    __slots__ = () # no __dict__ - that would be redundant

    @staticmethod # because this doesn't make sense as a global function.
    def _process_args(mapping=(), **kwargs):
        if hasattr(mapping, items):
            mapping = getattr(mapping, items)()
        return ((ensure_lower(k), ensure_to_base_units(v)) for k, v in chain(mapping, getattr(kwargs, items)()))

    def __init__(self, mapping=(), **kwargs):
        super(TransformedDict, self).__init__(self._process_args(mapping, **kwargs))

    def __getitem__(self, k):
        return super(TransformedDict, self).__getitem__(ensure_lower(k))

    def __setitem__(self, k, v):
        return super(TransformedDict, self).__setitem__(ensure_lower(k), ensure_to_base_units(v))

    def __delitem__(self, k):
        return super(TransformedDict, self).__delitem__(ensure_lower(k))

    def get(self, k, default=None):
        return super(TransformedDict, self).get(ensure_lower(k), default)

    def setdefault(self, k, default=None):
        return super(TransformedDict, self).setdefault(ensure_lower(k), default)

    def pop(self, k, v=_RaiseKeyError):
        if v is _RaiseKeyError:
            return super(TransformedDict, self).pop(ensure_lower(k))
        return super(TransformedDict, self).pop(ensure_lower(k), v)

    def update(self, mapping=(), **kwargs):
        super(TransformedDict, self).update(self._process_args(mapping, **kwargs))

    def __contains__(self, k):
        return super(TransformedDict, self).__contains__(ensure_lower(k))

    def copy(self): # don't delegate w/ super - dict.copy() -> dict :(
        return type(self)(self)

    @classmethod
    def fromkeys(cls, keys, v=None):
        return super(TransformedDict, cls).fromkeys((ensure_lower(k) for k in keys), v)

    def _repr_html_(self):
        attributes  = OrderedDict()
        attributes["[mass]"] = self["[mass]"]
        attributes["[length]"] = self["[length]"]
        attributes["[temperature]"] = self["[temperature]"]
        attributes["[time]"] = self["[time]"]
        attributes["[substance]"] = self["[substance]"]
        header = "<table>"
        footer = "</table>"
        html = ""
        for key, val in attributes.items():
            html += "<tr><td>{0}</td><td>{1}</td></tr>".format(key, val)

        return header + html + footer
