import gzip
import sys

with open(sys.argv[1], "rb") as f_in:
	with open(sys.argv[2], "wt") as f_out:
		f_out.write("#pragma once\n")
		f_out.write("#include <string_view>\n")

		name = sys.argv[1].replace('/', '_').replace('.', '_')
		f_out.write("constexpr std::string_view " + name + "_gz{\"");

		if sys.version_info >= (3,8):
			data = gzip.compress(f_in.read(), 9, mtime=0)
		else:
			data = gzip.compress(f_in.read(), 9)

		f_out.write("".join(map(lambda b: "\\x{0:02X}".format(b), data)))

		f_out.write(f"\", {len(data)}}};\n")
