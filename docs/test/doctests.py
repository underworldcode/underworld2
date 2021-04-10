import underworld as uw
import pkgutil, doctest, sys
import os
# lets disable metrics for tests
os.environ["UW_NO_USAGE_METRICS"] = "1"
os.environ["DOCTEST"] = "1"

test_vis = True
try:
    import lavavu
except ImportError:
    test_vis = False

for module in [uw,]:
    modIter = pkgutil.walk_packages(path=module.__path__, prefix=module.__name__+'.')

    for itthing, modName, bs in modIter:
        mod=sys.modules.get(modName)
        if modName.startswith("underworld.visualisation") and (not test_vis):
            print("Not testing `{}` as `lavavu` not found.".format(modName))
            continue
        print("Testing "+modName)
        res = doctest.testmod(mod)
        if res.failed > 0:
            raise RuntimeError("Doctest failed")

