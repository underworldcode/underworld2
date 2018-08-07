import underworld as uw
import glucifer
import pkgutil, doctest, sys
import os
# lets disable metrics for tests
os.environ["UW_NO_USAGE_METRICS"] = "1"
os.environ["DOCTEST"] = "1"

for module in [uw,glucifer]:
    modIter = pkgutil.walk_packages(path=module.__path__, prefix=module.__name__+'.')

    for itthing, modName, bs in modIter:
        mod=sys.modules.get(modName)
        print("Testing "+modName)
        res = doctest.testmod(mod)
        if res.failed > 0:
            raise RuntimeError("Doctest failed")

