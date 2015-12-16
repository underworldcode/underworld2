import underworld as uw
import pkgutil, doctest, sys

modIter = pkgutil.walk_packages(path=uw.__path__, prefix=uw.__name__+'.')

print sys.modules.get('underworld.container')

somethingFailed=False
for itthing, modName, bs in modIter:
    mod=sys.modules.get(modName)
    print "Testing "+modName,
    try:
        res = doctest.testmod(mod)
        if res.failed > 0:
            somethingFailed=True
            print " ... FAILED"
        else:
            print " ... pass"
    except:
        print "Died on" + modName + "\n"
        somethingFailed=True

if somethingFailed==True:
    sys.exit(1)

