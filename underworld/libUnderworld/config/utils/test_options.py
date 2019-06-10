import options

set = options.OptionSet()
set.add_option([options.Option("output", ["-o", "-beh"], sep=["", " "]),
                options.Option("blah", "-behemoth"),
                options.Option("flag", "-f", type="bool")])
s = set.make_option_string([("flag", True), ("output", "hello")])
print s
print set.parse_option_string("-o hello -behemoth -f -beh_h there")
