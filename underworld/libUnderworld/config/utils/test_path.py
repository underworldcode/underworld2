import path

# for p in path.find("dl", ["/usr/lib", "/lib"], "lib", [".so", ".so.2", ".a"]):
#     print p


print path.make_temp_name()
print path.make_temp_name()

print path.make_temp_name(prefix="tmp", ext=".so")

print path.make_temp_file(contents="hello\n", prefix="tmp", ext=".txt")

r = path.make_temp_file(contents="hello\n", prefix="tmp", ext=".txt", open=True)
print r
r[1].close()
