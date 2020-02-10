from . import conv, macro

class Environment(object):
    """Substitution environment used for storing and expanding arbitrary
    macros. Can store python functions which will be evaluated suring
    expansion."""

    def __init__(self, **kw):
        # We have to store the dictionary on a new class type because
        # dictionary types are unhashable and we'll probably need to
        # hash an environment sooner or later.
        self._dict = {}
        for k, v in kw.iteritems():
            self._dict[k] = v

    def __getitem__(self, k):
        return self._dict[k]

    def __setitem__(self, k, v):
        self._dict[k] = v

    def __delitem__(self, k):
        del self._dict[k]

    def __contains__(self, k):
        return k in self._dict

    def iterkeys(self):
        return self._dict.iterkeys()

    def iteritems(self):
        return self._dict.iteritems()

    def get(self, k, d):
        return self._dict.get(k, d)

    def update(self, env):
        for k, v in env.iteritems():
            self[k] = v

    def clear(self):
        self._dict = {}

    def dup(self):
        return Environment()

    def clone(self):
        env = self.dup()
        env.copy(self)
        return env

    def copy(self, env):
        self.clear()
        self.copy_dict(env, self)

    def copy_dict(self, src, dst):
        for k, v in src.iteritems():
            if isinstance(v, list):
                dst[k] = list(v)
            elif isinstance(v, dict):
                dst[k] = {}
                self.copy_dict(v, dst[k])
            else:
                dst[k] = v

    def merge(self, env):
        for k, v in env.iteritems():
            if isinstance(v, list):
                self.append_unique(k, *v)
            else:
                self.append_unique(k, v)

    def backup(self, *args):
        bak = {}
        for k in args:
            if k in self:
                if isinstance(self[k], list):
                    bak[k] = list(self[k])
                else:
                    bak[k] = self[k]
            else:
                bak[k] = None
        return bak

    def restore(self, bak):
        for k, v in bak.iteritems():
            if v is not None:
                self[k] = v
            elif k in self:
                del self[k]

    def append(self, k, *args, **kw):
        args = conv.flatten(list(args))
        args = [a for a in args if a != ""]
        if isinstance(k, (dict, Environment)):
            self._handle_dict(k, self.append, **kw)
        else:
            if k in self:
                if not isinstance(self[k], list):
                    self[k] = conv.to_list(self[k])
                self[k] += list(args)
            elif len(args) == 1:
                if kw.get("make_list", False):
                    self[k] = [args[0]]
                else:
                    self[k] = args[0]
            elif len(args) > 1:
                self[k] = list(args)

    def append_unique(self, k, *args, **kw):
        args = conv.flatten(list(args))
        if isinstance(k, (dict, Environment)):
            self._handle_dict(k, self.append_unique, **kw)
        else:
            self.append(k, *self._make_unique(k, args), **kw)

    def prepend(self, k, *args, **kw):
        args = conv.flatten(list(args))
        args = [a for a in args if a != ""]
        if k in self:
            if not isinstance(self[k], list):
                self[k] = conv.to_list(self[k])
            self[k] = list(args) + self[k]
        elif len(args) == 1:
            if kw.get("make_list", False):
                self[k] = [args[0]]
            else:
                self[k] = args[0]
        elif len(args) > 1:
            self[k] = list(args)

    def prepend_unique(self, k, *args, **kw):
        args = conv.flatten(list(args))
        self.prepend(k, *self._make_unique(k, args), **kw)

    def _make_unique(self, k, args):
        if k not in self:
            return args
        cur = conv.to_list(self[k])
        new_args = []
        for v in args:
            if v not in cur:
                new_args.append(v)
        return new_args

    def _handle_dict(self, d, op, **kw):
        for k, v in d.iteritems():
            if isinstance(v, list):
                op(k, *v, **kw)
            else:
                op(k, v, **kw)

    def subst(self, text):
        return macro.subst(text, self)

    def __repr__(self):
        return repr(self._dict)
