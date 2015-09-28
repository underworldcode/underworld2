import conv

def gen_combinations(items, size, cur_set=[]):
    if len(cur_set) == size:
        yield cur_set
    elif len(items):
        for c in gen_combinations(items[1:], size, cur_set + [items[0]]):
            yield c
        for c in gen_combinations(items[1:], size, cur_set):
            yield c

def combine2(item_set, size=-1, cur_size=0):
    """Produce combinations of items from each item list in 'item_set'. Resulting
    combinations are limited in size by 'size'. If item set is not a list of lists,
    but rather a single list, item lists for following recursions will be generated
    from 'item_set' minus the item selected for the current iteration."""
    if cur_size == size:
        yield []
    elif len(item_set) > 1:
        if item_set[0]:
            for i in item_set[0]:
                if i is None:
                    for r in combine2(item_set[1:], size, cur_size):
                        yield r
                else:
                    for r in combine2(item_set[1:], size, cur_size + 1):
                        yield [i] + r
        else:
            for r in combine2(item_set[1:], size, cur_size):
                    yield r
    elif len(item_set) == 1:
        if item_set[0]:
            for i in item_set[0]:
                if i is None:
                    yield []
                else:
                    yield [i]
        else:
            yield []
    else:
        yield []

def combine(items):
    sets = []
    for i in items:
        sets.append([None, i])
    return [c for c in combine2(sets)]
#     items = conv.to_list(items)
#     combs = []
#     flags = [0 for i in range(len(items))]
#     num = 2 ** len(items) - 1
#     for i in range(num):
#         for j in range(len(flags), 0, -1):
#             if flags[j - 1] == 1:
#                 flags[j - 1] = 0
#             else:
#                 flags[j - 1] = 1
#                 break
#         cur_set = [items[j] for j in range(len(flags)) if flags[j]]
#         combs.append(cur_set)
#     return combs
