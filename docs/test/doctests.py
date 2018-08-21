import underworld as uw
import pkgutil, doctest, sys
import os
# lets disable metrics for tests
os.environ["UW_NO_USAGE_METRICS"] = "1"
os.environ["DOCTEST"] = "1"

modIter = pkgutil.walk_packages(path=uw.__path__, prefix=uw.__name__+'.')

for itthing, modName, bs in modIter:
    mod=sys.modules.get(modName)
    print("Testing "+modName)
    res = doctest.testmod(mod)
    if res.failed > 0:
        raise RuntimeError("Doctest failed")

