def build_next_tier(tiers, rem):
    all = []
    for t in tiers:
        all += t
    next = []
    to_del = []
    size = len(rem)
    for i in range(size):
        node, deps = rem[i]
        valid = True
        for d in deps:
            if d not in all:
                valid = False
                break
        if valid:
            next.append(node)
            to_del.append(i)
    to_del.reverse()
    for i in to_del:
        del rem[i]
    tiers.append(next)

def build_tiers(rem):
    tiers = []
    while len(rem):
        build_next_tier(tiers, rem)
        if len(tiers[-1]) == 0:
            raise "Cyclic graph: cannot build tiers."
    return tiers
