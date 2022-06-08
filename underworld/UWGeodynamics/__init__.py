from __future__ import print_function,  absolute_import
import warnings

import underworld as uw
from underworld import __version__
from underworld.scaling import get_coefficients
from underworld.scaling import units as UnitRegistry
from underworld.scaling import non_dimensionalise
from underworld.scaling import dimensionalise

scaling_coefficients = get_coefficients()
nd = non_dimensionalise
dim = dimensionalise
u = UnitRegistry

import sys
import os
import errno
import tempfile
import locale
import uuid as _uuid
from itertools import chain
from mpi4py import MPI as _MPI
from . import shapes
from . import surfaceProcesses
from . import utilities
from ._rcParams import rcParams as defaultParams
from .LecodeIsostasy import LecodeIsostasy
from .lithopress import Lithostatic_pressure
from ._rheology import Rheology, ConstantViscosity, ViscousCreep
from ._rheology import DruckerPrager, VonMises
from ._rheology import CompositeViscosity
from ._rheology import ViscousCreepRegistry, PlasticityRegistry
from ._rheology import Elasticity
from ._material import Material, MaterialRegistry
from ._density import ConstantDensity, LinearDensity
from ._melt import Solidus, Liquidus, SolidusRegistry, LiquidusRegistry
from ._utils import Balanced_InflowOutflow
from ._utils import circles_grid, fn_Tukey_window, circle_points_tracers, sphere_points_tracers
from ._utils import MovingWall
from ._utils import PhaseChange, WaterFill
from ._utils import extract_profile
from ._utils import remesh
from . import postprocessing

rank = uw.mpi.rank
size = uw.mpi.size
nProcs = size

__author__ = "Romain Beucher"
__copyright__ = "Copyright 2022, The Australian National University"
__credits__ = ["Romain Beucher",
               "Louis Moresi",
               "Julian Giordani",
               "John Mansour"]
__maintainer__ = "Romain Beucher"
__email__ = "romain.beucher@anu.edu.au"

_id = str(_uuid.uuid4())


def Dimensionalize(*args, **kwargs):
    import warnings
    warnings.warn("""'Dimensionalize' has been changed to 'dimensionalise', please use the later""")
    return dimensionalise(*args, **kwargs)


def nonDimensionalize(*args, **kwargs):
    import warnings
    warnings.warn("""'nonDimensionalize' has been changed to 'non_dimensionalise', please use the later""")
    return non_dimensionalise(*args, **kwargs)


def mkdirs(newdir, mode=0o777):
    """
    make directory *newdir* recursively, and set *mode*.  Equivalent to ::

        > mkdir -p NEWDIR
        > chmod MODE NEWDIR
    """
    os.makedirs(newdir, mode=mode, exist_ok=True)


def _is_writable_dir(p):
    """
    p is a string pointing to a putative writable dir -- return True p
    is such a string, else False
    """
    try:
        p + ''  # test is string like
    except TypeError:
        return False

    # Test whether the operating system thinks it's a writable directory.
    # Note that this check is necessary on Google App Engine, because the
    # subsequent check will succeed even though p may not be writable.
    if not os.access(p, os.W_OK) or not os.path.isdir(p):
        return False

    # Also test that it is actually possible to write to a file here.
    try:
        t = tempfile.TemporaryFile(dir=p)
        try:
            t.write(b'1')
        finally:
            t.close()
    except:
        return False

    return True

def get_home():
    """Find user's home directory if possible.
    Otherwise, returns None.

    :see:
        http://mail.python.org/pipermail/python-list/2005-February/325395.html
    """
    try:
        path = os.path.expanduser("~")
    except ImportError:
        # This happens on Google App Engine (pwd module is not present).
        pass
    else:
        if os.path.isdir(path):
            return path
    for evar in ('HOME', 'USERPROFILE', 'TMP'):
        path = os.environ.get(evar)
        if path is not None and os.path.isdir(path):
            return path
    return None

def _create_tmp_config_dir():
    """
    If the config directory can not be created, create a temporary
    directory.

    Returns None if a writable temporary directory could not be created.
    """
    import getpass

    try:
        tempdir = tempfile.gettempdir()
    except NotImplementedError:
        # Some restricted platforms (such as Google App Engine) do not provide
        # gettempdir.
        return None

    try:
        username = getpass.getuser()
    except KeyError:
        username = str(os.getuid())
    tempdir = os.path.join(tempdir, 'uwgeodynamics-%s' % username)

    os.environ['UWGEOCONFIGDIR'] = tempdir

    mkdirs(tempdir)

    return tempdir

def _get_xdg_config_dir():
    """
    Returns the XDG configuration directory, according to the `XDG
    base directory spec
    <http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html>`_.
    """
    path = os.environ.get('XDG_CONFIG_HOME')
    if path is None:
        path = get_home()
        if path is not None:
            path = os.path.join(path, '.config')
    return path

def _get_config_or_cache_dir(xdg_base):

    p = None
    h = get_home()
    if h is not None:
        p = os.path.join(h, '.uwgeodynamics')
    if (sys.platform.startswith('linux') and xdg_base):
        p = os.path.join(xdg_base, 'uwgeodynamics')

    if p is not None:
        if os.path.exists(p):
            if _is_writable_dir(p):
                return p
        else:
            try:
                mkdirs(p)
            except OSError:
                pass
            else:
                return p

    return _create_tmp_config_dir()

def _get_configdir():
    """
    Return the string representing the configuration directory.

    The directory is chosen as follows:

    1. If the MPLCONFIGDIR environment variable is supplied, choose that.

    2a. On Linux, if `$HOME/.matplotlib` exists, choose that, but warn that
        that is the old location.  Barring that, follow the XDG specification
        and look first in `$XDG_CONFIG_HOME`, if defined, or `$HOME/.config`.

    2b. On other platforms, choose `$HOME/.matplotlib`.

    3. If the chosen directory exists and is writable, use that as the
       configuration directory.
    4. If possible, create a temporary directory, and use it as the
       configuration directory.
    5. A writable directory could not be found or created; return None.
    """
    return _get_config_or_cache_dir(_get_xdg_config_dir())

def _decode_filesystem_path(path):
    if isinstance(path, bytes):
        return path.decode(sys.getfilesystemencoding())
    else:
        return path

def get_data_path():
    'get the path to matplotlib data'

    if 'UWGEODYNAMICSDATA' in os.environ:
        path = os.environ['UWGEODYNAMICSDATA']
        if not os.path.isdir(path):
            raise RuntimeError('Path in environment UWGEODYNAMICSDATA not a '
                               'directory')
        return path

    _file = _decode_filesystem_path(__file__)
    path = os.sep.join([os.path.dirname(_file), 'uwgeo-data'])
    if os.path.isdir(path):
        return path

    raise RuntimeError('Could not find the UWGeodynamics data files')

def uwgeodynamics_fname():
    """
    Get the location of the config file.

    The file location is determined in the following order

    - `$PWD/uwgeodynamicsrc`

    - `$UWGEODYNAMICSRC` if it is a file

    - `$UWGEODYNAMICSRC/uwgeodynamicsrc`

    - `$MPLCONFIGDIR/uwgeodynamicsrc`

    - On Linux,

          - `$HOME/.uwgeodynamics/uwgeodynamicsrc`, if it exists

          - or `$XDG_CONFIG_HOME/uwgeodynamics/uwgeodynamicsrc` (if
            $XDG_CONFIG_HOME is defined)

          - or `$HOME/.config/uwgeodynamics/uwgeodynamicsrc` (if
            $XDG_CONFIG_HOME is not defined)

    - On other platforms,

         - `$HOME/.uwgeodynamics/uwgeodynamicsrc` if `$HOME` is defined.

    - Lastly, it looks in `$UWGEODYNAMICSDATA/uwgeodynamicsrc` for a
      system-defined copy.
    """
    cwd = os.getcwd()
    fname = os.path.join(cwd, 'uwgeodynamicsrc')
    if os.path.exists(fname):
        return fname

    if 'UWGEODYNAMICSRC' in os.environ:
        path = os.environ['UWGEODYNAMICSRC']
        if os.path.exists(path):
            if os.path.isfile(path):
                return path
            fname = os.path.join(path, 'uwgeodynamicsrc')
            if os.path.exists(fname):
                return fname

    configdir = _get_configdir()
    if configdir is not None:
        fname = os.path.join(configdir, 'uwgeodynamicsrc')
        if os.path.exists(fname):
            home = get_home()
            if (sys.platform.startswith('linux') and
                home is not None and
                os.path.exists(os.path.join(
                    home, '.uwgeodynamics', 'uwgeodynamicsrc'))):
                warnings.warn(
                    "Found UWGeodynamics configuration in ~/.uwgeodynamics/. "
                    "To conform with the XDG base directory standard, "
                    "this configuration location has been deprecated "
                    "on Linux, and the new location is now %s/uwgeodynamics/. "
                    "Please move your configuration there to ensure that "
                    "UWGeodynamics will continue to find it in the future." %
                    _get_xdg_config_dir())
                return os.path.join(
                    home, '.uwgeodynamics', 'uwgeodynamicsrc')
            return fname

    path = get_data_path()  # guaranteed to exist or raise
    fname = os.path.join(path, 'uwgeodynamicsrc')
    if not os.path.exists(fname):
        warnings.warn('Could not find matplotlibrc; using defaults')

    return fname

# names of keys to deprecate
# the values are a tuple of (new_name, f_old_2_new, f_new_2_old)
# the inverse function may be `None`
_deprecated_map = {
    }

_deprecated_ignore_map = {
    }

_deprecated_abort = {
}

_obsolete_set = set()
_all_deprecated = set(chain(_deprecated_ignore_map,
                            _deprecated_map, _obsolete_set))

class RcParams(dict):

    """
    A dictionary object including validation

    validating functions are defined and associated with rc parameters in
    :mod:`matplotlib.rcsetup`
    """

    validate = dict((key, converter) for key, (default, converter) in
                    defaultParams.items()
                    if key not in _all_deprecated)
    msg_depr = "%s is deprecated and replaced with %s; please use the latter."
    msg_depr_ignore = "%s is deprecated and ignored. Use %s"

    # validate values on the way in
    def __init__(self, *args, **kwargs):
        for k, v in dict(*args, **kwargs).items():
            self[k] = v

    def __setitem__(self, key, val):
        try:
            if key in _deprecated_map:
                alt_key, alt_val, _ = _deprecated_map[key]
                warnings.warn(self.msg_depr % (key, alt_key))
                key = alt_key
                val = alt_val(val)
            elif key in _deprecated_ignore_map:
                alt = _deprecated_ignore_map[key]
                warnings.warn(self.msg_depr_ignore % (key, alt))
                return
            elif key in _deprecated_abort:
                alt = _deprecated_abort[key]
                raise ValueError(self.msg_depr % (key, alt))
            try:
                cval = self.validate[key](val)
            except ValueError as ve:
                raise ValueError("Key %s: %s" % (key, str(ve)))
            dict.__setitem__(self, key, cval)
        except KeyError:
            raise KeyError('%s is not a valid rc parameter.\
See rcParams.keys() for a list of valid parameters.' % (key,))

    def __getitem__(self, key):
        inverse_alt = None
        if key in _deprecated_map:
            alt_key, _, inverse_alt = _deprecated_map[key]
            warnings.warn(self.msg_depr % (key, alt_key))
            key = alt_key

        elif key in _deprecated_ignore_map:
            alt = _deprecated_ignore_map[key]
            warnings.warn(self.msg_depr_ignore % (key, alt))
            key = alt
        elif key in _deprecated_abort:
            alt = _deprecated_abort[key]
            raise ValueError(self.msg_depr % (key, alt))

        val = dict.__getitem__(self, key)
        if inverse_alt is not None:
            return inverse_alt(val)

        return val

    # http://stackoverflow.com/questions/2390827
    # (how-to-properly-subclass-dict-and-override-get-set)
    # the default dict `update` does not use __setitem__
    # so rcParams.update(...) (such as in seaborn) side-steps
    # all of the validation over-ride update to force
    # through __setitem__
    def update(self, *args, **kwargs):
        for k, v in dict(*args, **kwargs).items():
            self[k] = v

    def __repr__(self):
        import pprint
        class_name = self.__class__.__name__
        indent = len(class_name) + 1
        repr_split = pprint.pformat(dict(self), indent=1,
                                    width=80 - indent).split('\n')
        repr_indented = ('\n' + ' ' * indent).join(repr_split)
        return '{0}({1})'.format(class_name, repr_indented)

    def __str__(self):
        return '\n'.join('{0}: {1}'.format(k, v)
                         for k, v in sorted(self.items()))

    def keys(self):
        """
        Return sorted list of keys.
        """
        k = list(dict.keys(self))
        k.sort()
        return k

    def values(self):
        """
        Return values in order of sorted keys.
        """
        return [self[k] for k in self.keys()]

    def find_all(self, pattern):
        """
        Return the subset of this RcParams dictionary whose keys match,
        using :func:`re.search`, the given ``pattern``.

        .. note::

            Changes to the returned dictionary are *not* propagated to
            the parent RcParams dictionary.

        """
        import re
        pattern_re = re.compile(pattern)
        return RcParams((key, value)
                        for key, value in self.items()
                        if pattern_re.search(key))


def rc_params(fail_on_error=False):
    """Return a :class:`uwgeodynamics.RcParams` instance from the
    default uwgeodynamics rc file.
    """
    fname = uwgeodynamics_fname()
    if not os.path.exists(fname):
        # this should never happen, default in mpl-data should always be found
        message = 'could not find rc file; returning defaults'
        ret = RcParams([(key, default) for key, (default, _) in
                        defaultParams.items()
                        if key not in _all_deprecated])
        warnings.warn(message)
        return ret

    return rc_params_from_file(fname, fail_on_error)

_error_details_fmt = 'line #%d\n\t"%s"\n\tin file "%s"'


def _rc_params_in_file(fname, fail_on_error=False):
    """Return :class:`matplotlib.RcParams` from the contents of the given file.

    Unlike `rc_params_from_file`, the configuration class only contains the
    parameters specified in the file (i.e. default values are not filled in).
    """
    cnt = 0
    rc_temp = {}
    with open(fname, "r") as fd:
        try:
            for line in fd:
                cnt += 1
                strippedline = line.split('#', 1)[0].strip()
                if not strippedline:
                    continue
                tup = strippedline.split(':', 1)
                if len(tup) != 2:
                    error_details = _error_details_fmt % (cnt, line, fname)
                    warnings.warn('Illegal %s' % error_details)
                    continue
                key, val = tup
                key = key.strip()
                val = val.strip()
                if key in rc_temp:
                    warnings.warn('Duplicate key in file "%s", line #%d' %
                                  (fname, cnt))
                rc_temp[key] = (val, line, cnt)
        except UnicodeDecodeError:
            warnings.warn(
                ('Cannot decode configuration file %s with '
                 'encoding %s, check LANG and LC_* variables')
                % (fname, locale.getdefaultlocale()[1] or 'utf-8 (default)'))
            raise

    config = RcParams()

    for key, (val, line, cnt) in rc_temp.items():
        if key in defaultParams:
            if fail_on_error:
                config[key] = val  # try to convert to proper type or raise
            else:
                try:
                    config[key] = val  # try to convert to proper type or skip
                except Exception as msg:
                    error_details = _error_details_fmt % (cnt, line, fname)
                    warnings.warn('Bad val "%s" on %s\n\t%s' %
                                  (val, error_details, msg))
        elif key in _deprecated_ignore_map:
            warnings.warn('%s is deprecated. Update your matplotlibrc to use '
                          '%s instead.' % (key, _deprecated_ignore_map[key]))

        else:
            print(""" Bad key "%s" on line %d in %s.""" % (key, cnt, fname))

    return config


def rc_params_from_file(fname, fail_on_error=False, use_default_template=True):
    """Return :class:`matplotlib.RcParams` from the contents of the given file.

    Parameters
    ----------
    fname : str
        Name of file parsed for matplotlib settings.
    fail_on_error : bool
        If True, raise an error when the parser fails to convert a parameter.
    use_default_template : bool
        If True, initialize with default parameters before updating with those
        in the given file. If False, the configuration class only contains the
        parameters specified in the file. (Useful for updating dicts.)
    """
    config_from_file = _rc_params_in_file(fname, fail_on_error)

    if not use_default_template:
        return config_from_file

    iter_params = defaultParams.items()
    config = RcParams([(key, default) for key, (default, _) in iter_params
                                      if key not in _all_deprecated])
    config.update(config_from_file)

    if rank == 0:
        print('loaded rc file %s' % fname)
        sys.stdout.flush()

    return config


def _in_doctest():
    """
    Returns true if running inside a doctest.

    http://stackoverflow.com/questions/8116118/how-to-determine-whether-code-is-running-in-a-doctest
    """
    return hasattr(sys.modules['__main__'], '_SpoofOut')

rcParams = rc_params()

rcParamsOrig = rcParams.copy()

rcParamsDefault = defaultParams

from ._model import Model
