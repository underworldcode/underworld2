import macro

def three(env, i1, i2):
    return str(int(i1) + int(i2))

def five(env, i1):
    return str(i1)

def eight(env, l):
    return str(reduce(lambda x,y: int(x)+int(y), l, 0))

env = {"one": "ten", "two": "twenty", "three": three, "four": "{$one}",
       "five": five, "six": ["hi", "there"], "seven": ["1", "2"], "eight": eight}

print macro.subst("$eight($seven)", env)

print macro.subst("{$three($five(30),20)}", env)

print macro.subst("$one + {$two} = thirty", env)

print macro.subst("$one + $two = {$three(10,20)}", env)

print macro.subst("$four", env)
