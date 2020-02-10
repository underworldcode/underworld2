def to_list(var):
    if isinstance(var, str):
        return [var]
    elif var is None:
        return []
    elif isinstance(var, list):
        return var
    elif isinstance(var, tuple):
        return list(var)
    else:
        return [var]

def flatten(var):
    if isinstance(var, list):
        sum = []
        for i in var:
            sum.extend(flatten(i))
        return sum
    else:
        return [var]
